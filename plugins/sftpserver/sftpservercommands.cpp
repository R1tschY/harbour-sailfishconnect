#include "sftpservercommands.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <algorithm>
#include <memory>

namespace {

int SSH_FX_NOT_A_DIRECTORY = 19;
int SSH_FX_INVALID_FILENAME = 20;

int READ_DIR_CHUNK_SIZE = 100;

template <typename T>
T* findHandle(sftp_client_message msg, const std::unordered_map<void*, std::unique_ptr<T>>& handles)
{
    void* handle = sftp_handle(msg->sftp, msg->handle);
    auto iter = handles.find(handle);
    if (iter == handles.end())
        return nullptr;
    return iter->second.get();
}

int toUnixPermissions(QFile::Permissions perms)
{
    return (perms & 0xF) | ((perms & 0xF0) >> 4) | ((perms & 0xF00) >> 8);
}

QString createLongName(const QFileInfo& file_info, const QString& filename)
{
    QString out;
    auto mode = file_info.permissions();

    if (file_info.isSymLink())
        out.append(QChar::fromLatin1('l'));
    else if (file_info.isDir())
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

    /*group*/
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

    out.append(QStringLiteral(" 1 %1 %2 %3").arg(file_info.ownerId()).arg(file_info.groupId()).arg(file_info.size()));
    out.append(QChar::fromLatin1(' '));
    out.append(file_info.lastModified().toString(QStringLiteral("MMM d hh:mm:ss yyyy")));
    out.append(QChar::fromLatin1(' '));
    out.append(filename);
    return out;
}

bool replyUnsupported(sftp_client_message msg)
{
    int type = sftp_client_message_get_type(msg);
    qDebug() << "SFTP: UNSUPPORTED" << type;
    return sftp_reply_status(msg, SSH_FX_OP_UNSUPPORTED, "Unsupported message");
}

bool replyOk(sftp_client_message msg)
{
    return sftp_reply_status(msg, SSH_FX_OK, nullptr);
}

bool replyFileError(sftp_client_message msg, const QFile& file) {
    qDebug() << "SFTP: FILE ERROR" << file.fileName() << file.error() << file.errorString();
    return sftp_reply_status(msg, SSH_FX_FAILURE, file.errorString().toUtf8().data());
}

} // namespace

SftpServerSession::SftpServerSession(sftp_session sftp)
    : m_sftpSession(sftp)
    , m_cwd(QStringLiteral("/home"))
{
}

int SftpServerSession::init() {
    printf("init SFTP %p\n", m_sftpSession.get());
    return sftp_server_init(m_sftpSession.get());
}

bool SftpServerSession::readMessage()
{
    sftp_client_message msg = sftp_get_client_message(m_sftpSession.get());
    if (msg == nullptr) {
        printf("Error sftp_get_client_message: %s\n", ssh_get_error(m_sftpSession->session));
        return false;
    }

    int type = sftp_client_message_get_type(msg);
    printf("read %p %d\n", m_sftpSession.get(), type);
    switch (type) {
    case SSH_FXP_READLINK:
        replyReadLink(msg);
        break;
    case SSH_FXP_REALPATH:
        replyRealPath(msg);
        break;
    case SSH_FXP_STAT:
        replyStat(msg, false);
        break;
    case SSH_FXP_LSTAT:
        replyStat(msg, true);
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
    
    case SSH_FXP_SETSTAT:

    case SSH_FXP_FSTAT:
    case SSH_FXP_FSETSTAT:

    case SSH_FXP_EXTENDED:
    default:
        replyUnsupported(msg);
        break;
    }

    sftp_client_message_free(msg);
    printf("/read %p\n", m_sftpSession.get());
    return true;
}

QString SftpServerSession::getFilename(const char* rawFilename)
{
    QString filename = QString::fromUtf8(rawFilename);
    if (filename.isEmpty()) {
        return QDir::homePath();
    }

    QFileInfo fileInfo{ filename };
    if (fileInfo.isRelative()) {
        return m_cwd.absoluteFilePath(filename);
    }

    return filename;
}

QString SftpServerSession::getFilename(sftp_client_message msg)
{
    return getFilename(sftp_client_message_get_filename(msg));
}

QString SftpServerSession::getDataFilename(sftp_client_message msg)
{
    return getFilename(sftp_client_message_get_data(msg));
}

sftp_attributes_struct SftpServerSession::getFileAttrs(const QFileInfo& fileInfo)
{
    sftp_attributes_struct attr{};

    attr.size = fileInfo.size();

    attr.uid = fileInfo.ownerId();
    attr.gid = fileInfo.groupId();

    attr.permissions = toUnixPermissions(fileInfo.permissions());
    attr.atime = fileInfo.lastRead().toUTC().toMSecsSinceEpoch() / 1000;
    attr.mtime = fileInfo.lastModified().toUTC().toMSecsSinceEpoch() / 1000;
    attr.flags = SSH_FILEXFER_ATTR_SIZE
        | SSH_FILEXFER_ATTR_UIDGID
        | SSH_FILEXFER_ATTR_PERMISSIONS
        | SSH_FILEXFER_ATTR_ACMODTIME;

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
    qDebug() << "SFTP: READLINK" << filename;

    if (filename.isEmpty()) {
        return sftp_reply_status(msg, SSH_FX_INVALID_FILENAME, "empty filename");
    }

    auto link = QFile::symLinkTarget(filename);
    if (link.isEmpty())
        return sftp_reply_status(msg, SSH_FX_NO_SUCH_FILE, "invalid link");

    sftp_attributes_struct attr{};
    sftp_reply_names_add(msg, link.toUtf8().data(), link.toUtf8().data(), &attr);
    return sftp_reply_names(msg) == 0;
}

bool SftpServerSession::replyRealPath(sftp_client_message msg)
{
    auto filename = getFilename(msg);
    // TODO: optional args: control-byte, compose-path

    qDebug() << "SFTP: REALPATH" << filename;
    if (filename.isEmpty()) {
        filename = m_cwd.path();
    }

    QString realPath = QFileInfo(filename).canonicalFilePath();
    if (realPath.isEmpty()) {
        return sftp_reply_status(msg, SSH_FX_NO_SUCH_FILE, "invalid file");
    }

    sftp_attributes_struct attr{};
    sftp_reply_names_add(msg, realPath.toUtf8().data(), realPath.toUtf8().data(), &attr);
    return sftp_reply_names(msg) == 0;
}

bool SftpServerSession::replyStat(sftp_client_message msg, bool follow)
{
    auto filename = getFilename(msg);

    qDebug().noquote() << "SFTP: STAT" << filename << follow;

    QFileInfo fileInfo(filename);
    if (!fileInfo.isSymLink() && !fileInfo.exists())
        return sftp_reply_status(msg, SSH_FX_NO_SUCH_FILE, "no such file");

    sftp_attributes_struct attr = {};
    if (!follow && fileInfo.isSymLink()) {
        // TODO
        return sftp_reply_status(msg, SSH_FX_OP_UNSUPPORTED, "TODO: links");
    } else {
        if (fileInfo.isSymLink())
            fileInfo = QFileInfo(fileInfo.symLinkTarget());
        
        attr = getFileAttrs(fileInfo);
    }

    return sftp_reply_attr(msg, &attr);
}

bool SftpServerSession::replyOpenDir(sftp_client_message msg)
{
    auto filename = getFilename(msg);

    qDebug() << "SFTP: OPEN DIR" << filename;
    if (filename.isEmpty()) {
        return sftp_reply_status(msg, SSH_FX_INVALID_FILENAME, "empty filename");
    }

    QFileInfo fileInfo{ filename };
    if (!fileInfo.exists())
        return sftp_reply_status(msg, SSH_FX_NO_SUCH_FILE, "no such directory");
    if (!fileInfo.isDir())
        return sftp_reply_status(msg, SSH_FX_NOT_A_DIRECTORY, "no such directory");
    if (!fileInfo.isReadable())
        return sftp_reply_status(msg, SSH_FX_PERMISSION_DENIED, "permission denied");

    auto iterator_handle = std::unique_ptr<QDirIterator>(new QDirIterator(filename));
    void* handle = iterator_handle.get();
    m_openDirHandles.emplace(handle, std::move(iterator_handle));

    SshStringPtr sftp_handle{ sftp_handle_alloc(m_sftpSession.get(), handle) };
    qDebug() << "SFTP: NEW OPENDIR HANDLE" << filename << sftp_handle.get();
    return sftp_reply_handle(msg, sftp_handle.get());
}

bool SftpServerSession::replyReadDir(sftp_client_message msg)
{
    auto* iterator = findHandle(msg, m_openDirHandles);
    qDebug() << "SFTP: READ DIR" << iterator;

    if (!iterator)
        return sftp_reply_status(msg, SSH_FX_INVALID_HANDLE, "invalid handle");

    int i = 0;
    for (; i < READ_DIR_CHUNK_SIZE && iterator->hasNext(); ++i) {
        QFileInfo entry = iterator->next();
        auto attr = getFileAttrs(entry);
        QString longName = createLongName(entry, entry.fileName());

        QByteArray cFilename = entry.fileName().toUtf8();
        sftp_reply_names_add(msg, cFilename.data(), longName.toUtf8().data(), &attr);
        qDebug().noquote() << "SFTP: READ DIR: res +=" << longName;
    }

    if (i == 0) {
        qDebug() << "SFTP: READ DIR" << iterator << "EOF";
        return sftp_reply_status(msg, SSH_FX_EOF, nullptr);
    }

    return sftp_reply_names(msg);
}

bool SftpServerSession::replyClose(sftp_client_message msg)
{
    void* handle = sftp_handle(m_sftpSession.get(), msg->handle);
    qDebug() << "SFTP: CLOSE HANDLE" << handle;

    if (!m_openFileHandles.erase(handle) && !m_openDirHandles.erase(handle))
        return sftp_reply_status(msg, SSH_FX_INVALID_HANDLE, "invalid handle");

    sftp_handle_remove(m_sftpSession.get(), handle);
    return replyOk(msg);
}

bool SftpServerSession::replyOpenFile(sftp_client_message msg)
{
    auto filename = getFilename(msg);

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

    // open
    qDebug() << "SFTP: OPEN FILE" << filename << flags << mode;
    auto file = std::make_unique<QFile>(filename);
    if (!file->open(mode)) {
        return replyFileError(msg, *file);
    }

    // TODO: set permissions
    void* handle = file.get();
    m_openFileHandles.emplace(handle, std::move(file));

    SshStringPtr sftp_handle{ sftp_handle_alloc(m_sftpSession.get(), handle) };
    return sftp_reply_handle(msg, sftp_handle.get());
}

bool SftpServerSession::replyReadFile(sftp_client_message msg)
{
    auto* file = findHandle(msg, m_openFileHandles);
    if (!file)
        return sftp_reply_status(msg, SSH_FX_INVALID_HANDLE, "invalid handle");

    size_t size = std::min(msg->len, 65536U * 128);

    if (!file->seek(msg->offset)) {
        return replyFileError(msg, *file);
    }

    std::vector<char> buffer;
    buffer.reserve(size);
    auto bytes = file->read(buffer.data(), size);
    if (bytes < 0) {
        return replyFileError(msg, *file);
    }
    if (bytes == 0 && size != 0) {
        qDebug() << "SFTP: READ FILE" << file << msg->offset << size << "EOF";
        return sftp_reply_status(msg, SSH_FX_EOF, nullptr);
    }

    qDebug() << "SFTP: READ FILE" << file << msg->offset << size;
    return sftp_reply_data(msg, buffer.data(), bytes);
}

bool SftpServerSession::replyWriteFile(sftp_client_message msg)
{
    auto* file = findHandle(msg, m_openFileHandles);
    if (!file)
        return sftp_reply_status(msg, SSH_FX_INVALID_HANDLE, "invalid handle");

    size_t size = ssh_string_len(msg->data);
    const char* data = ssh_string_get_char(msg->data);
    qDebug() << "SFTP: WRITE FILE" << file << msg->offset << size;

    if (!file->seek(msg->offset)) {
        return replyFileError(msg, *file);
    }

    while (size > 0) {
        auto bytes = file->write(data, size);
        if (bytes < 0) {
            return replyFileError(msg, *file);
        }

        size -= bytes;
        data += bytes;
    }

    return replyOk(msg);
}

bool SftpServerSession::replyRemove(sftp_client_message msg) 
{
    auto filename = getFilename(msg);
    qDebug() << "SFTP: REMOVE FILE" << filename;

    QFile file(filename);
    if (!file.remove()) {
        return replyFileError(msg, file);
    }

    return replyOk(msg);
}

bool SftpServerSession::replyMakeDir(sftp_client_message msg) 
{
    auto filename = getFilename(msg);
    qDebug() << "SFTP: MAKE DIR" << filename;

    QDir dir(filename);
    if (!dir.mkdir(filename)) {
        qDebug() << "SFTP: MAKE DIR FAILED" << dir;
        if (dir.exists()) {
            return sftp_reply_status(msg, SSH_FX_FILE_ALREADY_EXISTS, "directory already exists");    
        } else {
            return sftp_reply_status(msg, SSH_FX_FAILURE, "unknown error");
        }
    }
    
    return replyOk(msg);
}

bool SftpServerSession::replyRemoveDir(sftp_client_message msg) 
{
    auto filename = getFilename(msg);
    qDebug() << "SFTP: REMOVE DIR" << filename;

    QDir dir(filename);
    if (!dir.rmdir(filename)) {
        return sftp_reply_status(msg, SSH_FX_FAILURE, "unknown error");
    }
    
    return replyOk(msg);
}

bool SftpServerSession::replyRename(sftp_client_message msg) 
{
    auto source = getFilename(msg);
    auto target = getDataFilename(msg);
    qDebug() << "SFTP: RENAME FILE" << source << target;

    QFile sourceFile(source);
    QFile targetFile(target);

    if (!sourceFile.exists() && !QFileInfo(source).isSymLink()) {
        return sftp_reply_status(msg, SSH_FX_NO_SUCH_FILE, "no such file");
    }

    if (targetFile.exists()) {
        if (!targetFile.remove()) {
            return replyFileError(msg, targetFile);
        }
    }

    if (!sourceFile.rename(target)) {
        return replyFileError(msg, sourceFile);
    }
    
    return replyOk(msg);
}

bool SftpServerSession::replySymlink(sftp_client_message msg) 
{
    auto target = getFilename(msg);
    auto linkName = getDataFilename(msg);
    qDebug() << "SFTP: SYMLINK" << linkName << target;

    QFile targetFile(target);
    if (!targetFile.link(linkName)) {
        return replyFileError(msg, targetFile);
    }
    
    return replyOk(msg);
}

