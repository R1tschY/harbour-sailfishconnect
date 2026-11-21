/*
 * Copyright 2021 Richard Liebscher <richard.liebscher@gmail.com>.
 * Copyright 2026 Manuel FLURY <zx97@users.noreply.github.com>.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "sftpservercommands.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QString>

#include <algorithm>
#include <memory>
#include <vector>

Q_LOGGING_CATEGORY(loggerCommands, "kdeconnect.plugin.sftpserver")

namespace {

// Missing from libssh's headers.
constexpr int SSH_FX_NOT_A_DIRECTORY = 19;
constexpr int SSH_FX_INVALID_FILENAME = 20;

int READ_DIR_CHUNK_SIZE = 100;

/// Canonicalizes a path whose last components may not exist yet.
QString canonicalizePath(const QString &absolutePath)
{
    if (absolutePath.isEmpty()) {
        return QString();
    }

    QString existing = absolutePath;
    QStringList missingComponents;

    while (!existing.isEmpty() && existing != QLatin1String("/")
           && !QFileInfo::exists(existing) && !QFileInfo(existing).isSymLink()) {
        missingComponents.prepend(QFileInfo(existing).fileName());
        const QString parent = QFileInfo(existing).absolutePath();
        if (parent == existing) {
            break;
        }
        existing = parent;
    }

    QString canonical = QFileInfo(existing).canonicalFilePath();
    if (canonical.isEmpty()) {
        canonical = QDir::cleanPath(existing);
    }

    for (const QString &component : missingComponents) {
        canonical += QLatin1Char('/') + component;
    }

    return QDir::cleanPath(canonical);
}

template <typename T>
T *findHandle(sftp_client_message msg,
              const std::unordered_map<void *, std::unique_ptr<T>> &handles)
{
    void *handle = sftp_handle(msg->sftp, msg->handle);
    auto iter = handles.find(handle);
    if (iter == handles.end())
        return nullptr;
    return iter->second.get();
}

int toUnixPermissions(QFile::Permissions perms)
{
    int mode = 0;

    if (perms.testFlag(QFileDevice::ReadOwner))
        mode |= 0400;
    if (perms.testFlag(QFileDevice::WriteOwner))
        mode |= 0200;
    if (perms.testFlag(QFileDevice::ExeOwner))
        mode |= 0100;
    if (perms.testFlag(QFileDevice::ReadGroup))
        mode |= 0040;
    if (perms.testFlag(QFileDevice::WriteGroup))
        mode |= 0020;
    if (perms.testFlag(QFileDevice::ExeGroup))
        mode |= 0010;
    if (perms.testFlag(QFileDevice::ReadOther))
        mode |= 0004;
    if (perms.testFlag(QFileDevice::WriteOther))
        mode |= 0002;
    if (perms.testFlag(QFileDevice::ExeOther))
        mode |= 0001;

    return mode;
}

QFile::Permissions toQtPermissions(quint32 mode)
{
    QFile::Permissions permissions;

    if (mode & 0400)
        permissions |= QFileDevice::ReadOwner;
    if (mode & 0200)
        permissions |= QFileDevice::WriteOwner;
    if (mode & 0100)
        permissions |= QFileDevice::ExeOwner;
    if (mode & 0040)
        permissions |= QFileDevice::ReadGroup;
    if (mode & 0020)
        permissions |= QFileDevice::WriteGroup;
    if (mode & 0010)
        permissions |= QFileDevice::ExeGroup;
    if (mode & 0004)
        permissions |= QFileDevice::ReadOther;
    if (mode & 0002)
        permissions |= QFileDevice::WriteOther;
    if (mode & 0001)
        permissions |= QFileDevice::ExeOther;

    return permissions;
}

QString createLongName(const QFileInfo &fileInfo, const QString &filename)
{
    QString out;
    auto mode = fileInfo.permissions();

    if (fileInfo.isSymLink())
        out.append(QChar::fromLatin1('l'));
    else if (fileInfo.isDir())
        out.append(QChar::fromLatin1('d'));
    else
        out.append(QChar::fromLatin1('-'));

    /* user */
    if (mode & QFileDevice::ReadOwner)
        out.append(QChar::fromLatin1('r'));
    else
        out.append(QChar::fromLatin1('-'));

    if (mode & QFileDevice::WriteOwner)
        out.append(QChar::fromLatin1('w'));
    else
        out.append(QChar::fromLatin1('-'));

    if (mode & QFileDevice::ExeOwner)
        out.append(QChar::fromLatin1('x'));
    else
        out.append(QChar::fromLatin1('-'));

    /* group */
    if (mode & QFileDevice::ReadGroup)
        out.append(QChar::fromLatin1('r'));
    else
        out.append(QChar::fromLatin1('-'));

    if (mode & QFileDevice::WriteGroup)
        out.append(QChar::fromLatin1('w'));
    else
        out.append(QChar::fromLatin1('-'));

    if (mode & QFileDevice::ExeGroup)
        out.append(QChar::fromLatin1('x'));
    else
        out.append(QChar::fromLatin1('-'));

    /* other */
    if (mode & QFileDevice::ReadOther)
        out.append(QChar::fromLatin1('r'));
    else
        out.append(QChar::fromLatin1('-'));

    if (mode & QFileDevice::WriteOther)
        out.append(QChar::fromLatin1('w'));
    else
        out.append(QChar::fromLatin1('-'));

    if (mode & QFileDevice::ExeOther)
        out.append(QChar::fromLatin1('x'));
    else
        out.append(QChar::fromLatin1('-'));

    out.append(QStringLiteral(" 1 %1 %2 %3")
                   .arg(fileInfo.ownerId())
                   .arg(fileInfo.groupId())
                   .arg(fileInfo.size()));
    out.append(QChar::fromLatin1(' '));
    out.append(fileInfo.lastModified().toString(QStringLiteral("MMM d hh:mm:ss yyyy")));
    out.append(QChar::fromLatin1(' '));
    out.append(filename);
    return out;
}

bool replyUnsupported(sftp_client_message msg)
{
    int type = sftp_client_message_get_type(msg);
    qCDebug(loggerCommands) << "SFTP: UNSUPPORTED" << type;
    return sftp_reply_status(msg, SSH_FX_OP_UNSUPPORTED, "Unsupported message");
}

bool replyOk(sftp_client_message msg)
{
    return sftp_reply_status(msg, SSH_FX_OK, nullptr);
}

bool replyFileError(sftp_client_message msg, const QFile &file)
{
    qCWarning(loggerCommands) << "SFTP: FILE ERROR" << file.fileName()
                              << file.error() << file.errorString();
    return sftp_reply_status(msg, SSH_FX_FAILURE, file.errorString().toUtf8().data());
}

bool replyPathDenied(sftp_client_message msg)
{
    return sftp_reply_status(msg, SSH_FX_PERMISSION_DENIED,
                             "path outside of the exported directory");
}

} // namespace

SftpServerSession::SftpServerSession(sftp_session sftp, const QString &rootPath)
    : m_sftpSession(sftp)
    , m_rootPath(rootPath)
    , m_rootCanonicalPath(canonicalizePath(rootPath))
{
}

int SftpServerSession::init()
{
    return sftp_server_init(m_sftpSession.get());
}

bool SftpServerSession::readMessage()
{
    sftp_client_message msg = sftp_get_client_message(m_sftpSession.get());
    if (msg == nullptr) {
        qCDebug(loggerCommands) << "SFTP session ended:"
                                << ssh_get_error(m_sftpSession->session);
        return false;
    }

    int type = sftp_client_message_get_type(msg);
    qCDebug(loggerCommands) << "SFTP: message" << type;
    switch (type) {
    case SSH_FXP_READLINK:
        replyReadLink(msg);
        break;
    case SSH_FXP_REALPATH:
        replyRealPath(msg);
        break;
    case SSH_FXP_STAT:
        replyStat(msg, true);
        break;
    case SSH_FXP_LSTAT:
        replyStat(msg, false);
        break;
    case SSH_FXP_SETSTAT:
        replySetStat(msg, true);
        break;

    case SSH_FXP_OPENDIR:
        replyOpenDir(msg);
        break;
    case SSH_FXP_READDIR:
        replyReadDir(msg);
        break;

    case SSH_FXP_OPEN:
        replyOpenFile(msg);
        break;
    case SSH_FXP_READ:
        replyReadFile(msg);
        break;
    case SSH_FXP_WRITE:
        replyWriteFile(msg);
        break;

    case SSH_FXP_CLOSE:
        replyClose(msg);
        break;

    case SSH_FXP_REMOVE:
        replyRemove(msg);
        break;
    case SSH_FXP_MKDIR:
        replyMakeDir(msg);
        break;
    case SSH_FXP_RMDIR:
        replyRemoveDir(msg);
        break;
    case SSH_FXP_RENAME:
        replyRename(msg);
        break;
    case SSH_FXP_SYMLINK:
        replySymlink(msg);
        break;

    case SSH_FXP_FSTAT:
        replyFStat(msg);
        break;
    case SSH_FXP_FSETSTAT:
        replyFSetStat(msg);
        break;
    case SSH_FXP_EXTENDED:
        replyExtended(msg);
        break;
    default:
        replyUnsupported(msg);
        break;
    }

    sftp_client_message_free(msg);
    return true;
}

QString SftpServerSession::resolvePath(const QString &rawFilename) const
{
    // The SFTP namespace is virtual: "/" is the exported directory.
    QString virtualPath = rawFilename;
    if (virtualPath.isEmpty()) {
        virtualPath = QStringLiteral("/");
    }
    if (!virtualPath.startsWith(QLatin1Char('/'))) {
        virtualPath.prepend(QLatin1Char('/'));
    }
    virtualPath = QDir::cleanPath(virtualPath);
    if (virtualPath == QLatin1String(".")) {
        virtualPath = QStringLiteral("/");
    }

    const QString realPath = virtualPath == QLatin1String("/")
        ? m_rootPath
        : m_rootPath + virtualPath;

    if (!isInsideRoot(canonicalizePath(realPath))) {
        qCWarning(loggerCommands) << "SFTP: rejected path outside of export:"
                                  << rawFilename;
        return QString();
    }

    return realPath;
}

bool SftpServerSession::isInsideRoot(const QString &path) const
{
    if (m_rootCanonicalPath.isEmpty()) {
        return false;
    }
    if (path == m_rootCanonicalPath) {
        return true;
    }
    return path.startsWith(m_rootCanonicalPath + QLatin1Char('/'));
}

QString SftpServerSession::getFilename(const char *rawFilename)
{
    return resolvePath(QString::fromUtf8(rawFilename ? rawFilename : ""));
}

QString SftpServerSession::getFilename(sftp_client_message msg)
{
    return getFilename(sftp_client_message_get_filename(msg));
}

QString SftpServerSession::getDataFilename(sftp_client_message msg)
{
    return getFilename(sftp_client_message_get_data(msg));
}

sftp_attributes_struct SftpServerSession::getFileAttrs(const QFileInfo &fileInfo)
{
    sftp_attributes_struct attr{};

    attr.size = static_cast<uint64_t>(fileInfo.size());

    attr.uid = fileInfo.ownerId();
    attr.gid = fileInfo.groupId();

    attr.permissions = static_cast<uint32_t>(toUnixPermissions(fileInfo.permissions()));
    attr.atime = static_cast<uint32_t>(fileInfo.lastRead().toMSecsSinceEpoch() / 1000);
    attr.mtime = static_cast<uint32_t>(fileInfo.lastModified().toMSecsSinceEpoch() / 1000);
    attr.flags = SSH_FILEXFER_ATTR_SIZE | SSH_FILEXFER_ATTR_UIDGID
        | SSH_FILEXFER_ATTR_PERMISSIONS | SSH_FILEXFER_ATTR_ACMODTIME;

    if (fileInfo.isSymLink())
        attr.permissions |= SSH_S_IFLNK | 0777;
    else if (fileInfo.isDir())
        attr.permissions |= SSH_S_IFDIR;
    else if (fileInfo.isFile())
        attr.permissions |= SSH_S_IFREG;

    return attr;
}

bool SftpServerSession::replyReadLink(sftp_client_message msg)
{
    auto filename = getFilename(msg);
    if (filename.isEmpty()) {
        return replyPathDenied(msg);
    }

    auto link = QFile::symLinkTarget(filename);
    if (link.isEmpty())
        return sftp_reply_status(msg, SSH_FX_NO_SUCH_FILE, "invalid link");

    sftp_attributes_struct attr{};
    QByteArray linkUtf8 = link.toUtf8();
    sftp_reply_names_add(msg, linkUtf8.constData(), linkUtf8.constData(), &attr);
    return sftp_reply_names(msg) == 0;
}

bool SftpServerSession::replyRealPath(sftp_client_message msg)
{
    const QString requested = QString::fromUtf8(
        sftp_client_message_get_filename(msg));

    QString virtualPath = requested;
    if (virtualPath.isEmpty()) {
        virtualPath = QStringLiteral("/");
    }
    if (!virtualPath.startsWith(QLatin1Char('/'))) {
        virtualPath.prepend(QLatin1Char('/'));
    }
    virtualPath = QDir::cleanPath(virtualPath);
    if (virtualPath == QLatin1String(".")) {
        virtualPath = QStringLiteral("/");
    }

    const QString realPath = canonicalizePath(
        virtualPath == QLatin1String("/") ? m_rootPath : m_rootPath + virtualPath);
    if (!isInsideRoot(realPath)) {
        return replyPathDenied(msg);
    }

    const QString reportedPath = realPath == m_rootCanonicalPath
        ? QStringLiteral("/")
        : realPath.mid(m_rootCanonicalPath.size());
    QByteArray realPathUtf8 = reportedPath.toUtf8();
    sftp_attributes_struct attr{};
    sftp_reply_names_add(msg, realPathUtf8.constData(), realPathUtf8.constData(), &attr);
    return sftp_reply_names(msg) == 0;
}

bool SftpServerSession::replyStat(sftp_client_message msg, bool follow)
{
    auto filename = getFilename(msg);
    if (filename.isEmpty()) {
        return replyPathDenied(msg);
    }

    QFileInfo fileInfo(filename);
    if (!fileInfo.exists() && !fileInfo.isSymLink())
        return sftp_reply_status(msg, SSH_FX_NO_SUCH_FILE, "no such file");

    if (follow && fileInfo.isSymLink()) {
        const QString target = fileInfo.symLinkTarget();
        if (target.isEmpty()) {
            return sftp_reply_status(msg, SSH_FX_NO_SUCH_FILE, "broken link");
        }
        fileInfo = QFileInfo(target);
    }

    sftp_attributes_struct attr = getFileAttrs(fileInfo);
    return sftp_reply_attr(msg, &attr);
}

bool SftpServerSession::replySetStat(sftp_client_message msg, bool follow)
{
    auto filename = getFilename(msg);
    if (filename.isEmpty()) {
        return replyPathDenied(msg);
    }

    sftp_attributes attrs = msg->attr;
    if (!attrs) {
        return sftp_reply_status(msg, SSH_FX_FAILURE, "missing attributes");
    }

    if (attrs->flags & SSH_FILEXFER_ATTR_SIZE) {
        QFile file(filename);
        if (!file.resize(static_cast<qint64>(attrs->size))) {
            return replyFileError(msg, file);
        }
    }

    if (attrs->flags & SSH_FILEXFER_ATTR_PERMISSIONS) {
        QFile file(filename);
        if (!file.setPermissions(toQtPermissions(attrs->permissions))) {
            return replyFileError(msg, file);
        }
    }

    Q_UNUSED(follow)
    return replyOk(msg);
}

bool SftpServerSession::replyFStat(sftp_client_message msg)
{
    auto *file = findHandle(msg, m_openFileHandles);
    if (!file) {
        return sftp_reply_status(msg, SSH_FX_INVALID_HANDLE, "invalid handle");
    }

    QFileInfo fileInfo(file->fileName());
    sftp_attributes_struct attr = getFileAttrs(fileInfo);
    return sftp_reply_attr(msg, &attr);
}

bool SftpServerSession::replyFSetStat(sftp_client_message msg)
{
    auto *file = findHandle(msg, m_openFileHandles);
    if (!file) {
        return sftp_reply_status(msg, SSH_FX_INVALID_HANDLE, "invalid handle");
    }

    sftp_attributes attrs = msg->attr;
    if (!attrs) {
        return sftp_reply_status(msg, SSH_FX_FAILURE, "missing attributes");
    }

    if (attrs->flags & SSH_FILEXFER_ATTR_SIZE) {
        if (!file->resize(static_cast<qint64>(attrs->size))) {
            return replyFileError(msg, *file);
        }
    }

    if (attrs->flags & SSH_FILEXFER_ATTR_PERMISSIONS) {
        if (!QFile::setPermissions(file->fileName(),
                                   toQtPermissions(attrs->permissions))) {
            return replyFileError(msg, *file);
        }
    }

    return replyOk(msg);
}

bool SftpServerSession::replyExtended(sftp_client_message msg)
{
    const char *submessage = sftp_client_message_get_submessage(msg);
    if (submessage == nullptr) {
        return replyUnsupported(msg);
    }

    // sshfs uses the OpenSSH extensions advertised by libssh's
    // sftp_server_init(). Handle the ones that matter for daily use.
    if (qstrcmp(submessage, "posix-rename@openssh.com") == 0) {
        return replyRename(msg);
    }

    if (qstrcmp(submessage, "fsync@openssh.com") == 0) {
        return replyOk(msg);
    }

    qCDebug(loggerCommands) << "SFTP: unsupported extension" << submessage;
    return replyUnsupported(msg);
}

bool SftpServerSession::replyOpenDir(sftp_client_message msg)
{
    auto filename = getFilename(msg);
    if (filename.isEmpty()) {
        return replyPathDenied(msg);
    }

    QFileInfo fileInfo(filename);
    if (!fileInfo.exists())
        return sftp_reply_status(msg, SSH_FX_NO_SUCH_FILE, "no such directory");
    if (!fileInfo.isDir())
        return sftp_reply_status(msg, SSH_FX_NOT_A_DIRECTORY, "no such directory");
    if (!fileInfo.isReadable())
        return sftp_reply_status(msg, SSH_FX_PERMISSION_DENIED, "permission denied");

    auto iterator = std::unique_ptr<QDirIterator>(new QDirIterator(filename));
    void *handle = iterator.get();
    m_openDirHandles.emplace(handle, std::move(iterator));

    SshStringPtr sftpHandle{sftp_handle_alloc(m_sftpSession.get(), handle)};
    return sftp_reply_handle(msg, sftpHandle.get());
}

bool SftpServerSession::replyReadDir(sftp_client_message msg)
{
    auto *iterator = findHandle(msg, m_openDirHandles);
    if (!iterator)
        return sftp_reply_status(msg, SSH_FX_INVALID_HANDLE, "invalid handle");

    int i = 0;
    for (; i < READ_DIR_CHUNK_SIZE && iterator->hasNext(); ++i) {
        QFileInfo entry = iterator->next();
        auto attr = getFileAttrs(entry);
        QByteArray longName = createLongName(entry, entry.fileName()).toUtf8();
        QByteArray filename = entry.fileName().toUtf8();

        sftp_reply_names_add(msg, filename.constData(), longName.constData(), &attr);
    }

    if (i == 0) {
        return sftp_reply_status(msg, SSH_FX_EOF, nullptr);
    }

    return sftp_reply_names(msg) == 0;
}

bool SftpServerSession::replyClose(sftp_client_message msg)
{
    void *handle = sftp_handle(m_sftpSession.get(), msg->handle);
    if (!m_openFileHandles.erase(handle) && !m_openDirHandles.erase(handle))
        return sftp_reply_status(msg, SSH_FX_INVALID_HANDLE, "invalid handle");

    sftp_handle_remove(m_sftpSession.get(), handle);
    return replyOk(msg);
}

bool SftpServerSession::replyOpenFile(sftp_client_message msg)
{
    auto filename = getFilename(msg);
    if (filename.isEmpty()) {
        return replyPathDenied(msg);
    }

    // flags
    QIODevice::OpenMode mode = QIODevice::NotOpen;
    const auto flags = sftp_client_message_get_flags(msg);
    if (flags & SSH_FXF_READ)
        mode |= QIODevice::ReadOnly;
    if (flags & SSH_FXF_WRITE)
        mode |= QIODevice::WriteOnly;
    if (flags & SSH_FXF_APPEND)
        mode |= QIODevice::Append;
    if (flags & SSH_FXF_TRUNC)
        mode |= QIODevice::Truncate;
    if (flags & SSH_FXF_TEXT)
        mode |= QIODevice::Text;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
    if (flags & SSH_FXF_CREAT) {
        if (flags & SSH_FXF_EXCL)
            mode |= QIODevice::NewOnly;
    } else {
        mode |= QIODevice::ExistingOnly;
    }
#endif

    auto file = std::make_unique<QFile>(filename);
    if (!file->open(mode)) {
        return replyFileError(msg, *file);
    }

    void *handle = file.get();
    m_openFileHandles.emplace(handle, std::move(file));

    SshStringPtr sftpHandle{sftp_handle_alloc(m_sftpSession.get(), handle)};
    return sftp_reply_handle(msg, sftpHandle.get());
}

bool SftpServerSession::replyReadFile(sftp_client_message msg)
{
    auto *file = findHandle(msg, m_openFileHandles);
    if (!file)
        return sftp_reply_status(msg, SSH_FX_INVALID_HANDLE, "invalid handle");

    qint64 size = std::min<qint64>(msg->len, 65536 * 128);

    if (!file->seek(msg->offset)) {
        return replyFileError(msg, *file);
    }

    std::vector<char> buffer(static_cast<size_t>(size));
    auto bytes = file->read(buffer.data(), size);
    if (bytes < 0) {
        return replyFileError(msg, *file);
    }
    if (bytes == 0) {
        return sftp_reply_status(msg, SSH_FX_EOF, nullptr);
    }

    return sftp_reply_data(msg, buffer.data(), static_cast<int>(bytes));
}

bool SftpServerSession::replyWriteFile(sftp_client_message msg)
{
    auto *file = findHandle(msg, m_openFileHandles);
    if (!file)
        return sftp_reply_status(msg, SSH_FX_INVALID_HANDLE, "invalid handle");

    size_t size = ssh_string_len(msg->data);
    const char *data = ssh_string_get_char(msg->data);

    if (!file->seek(msg->offset)) {
        return replyFileError(msg, *file);
    }

    while (size > 0) {
        auto bytes = file->write(data, static_cast<qint64>(size));
        if (bytes < 0) {
            return replyFileError(msg, *file);
        }

        size -= static_cast<size_t>(bytes);
        data += bytes;
    }

    return replyOk(msg);
}

bool SftpServerSession::replyRemove(sftp_client_message msg)
{
    auto filename = getFilename(msg);
    if (filename.isEmpty()) {
        return replyPathDenied(msg);
    }

    QFile file(filename);
    if (!file.remove()) {
        return replyFileError(msg, file);
    }

    return replyOk(msg);
}

bool SftpServerSession::replyMakeDir(sftp_client_message msg)
{
    auto filename = getFilename(msg);
    if (filename.isEmpty()) {
        return replyPathDenied(msg);
    }

    const QFileInfo info(filename);
    QDir parent(info.absolutePath());
    if (!parent.mkdir(info.fileName())) {
        if (info.exists()) {
            return sftp_reply_status(msg, SSH_FX_FILE_ALREADY_EXISTS,
                                     "directory already exists");
        }
        return sftp_reply_status(msg, SSH_FX_FAILURE, "cannot create directory");
    }

    return replyOk(msg);
}

bool SftpServerSession::replyRemoveDir(sftp_client_message msg)
{
    auto filename = getFilename(msg);
    if (filename.isEmpty()) {
        return replyPathDenied(msg);
    }

    const QFileInfo info(filename);
    QDir parent(info.absolutePath());
    if (!parent.rmdir(info.fileName())) {
        return sftp_reply_status(msg, SSH_FX_FAILURE, "cannot remove directory");
    }

    return replyOk(msg);
}

bool SftpServerSession::replyRename(sftp_client_message msg)
{
    auto source = getFilename(msg);
    auto target = getDataFilename(msg);
    if (source.isEmpty() || target.isEmpty()) {
        return replyPathDenied(msg);
    }

    QFile sourceFile(source);
    if (!sourceFile.exists() && !QFileInfo(source).isSymLink()) {
        return sftp_reply_status(msg, SSH_FX_NO_SUCH_FILE, "no such file");
    }

    QFile targetFile(target);
    if (targetFile.exists() && !targetFile.remove()) {
        return replyFileError(msg, targetFile);
    }

    if (!sourceFile.rename(target)) {
        return replyFileError(msg, sourceFile);
    }

    return replyOk(msg);
}

bool SftpServerSession::replySymlink(sftp_client_message msg)
{
    auto linkPath = getFilename(msg);
    auto targetPath = getDataFilename(msg);
    if (linkPath.isEmpty() || targetPath.isEmpty()) {
        return replyPathDenied(msg);
    }

    QFile targetFile(targetPath);
    if (!targetFile.link(linkPath)) {
        return replyFileError(msg, targetFile);
    }

    return replyOk(msg);
}
