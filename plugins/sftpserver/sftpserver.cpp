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

#include "sftpserver.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QLoggingCategory>
#include <QThread>

#include <sys/socket.h>

#include <vector>

#include "sftpservercommands.h"
#include "ssh.h"

Q_LOGGING_CATEGORY(logger, "kdeconnect.plugin.sftpserver")

namespace {

constexpr int minPort = 40000;
constexpr int maxPort = 59999;
constexpr int maxPortTries = 40;
constexpr int maxPasswordTries = 3;

QByteArray randomBytes(int count)
{
    QFile urandom(QStringLiteral("/dev/urandom"));
    if (urandom.open(QIODevice::ReadOnly)) {
        const QByteArray bytes = urandom.read(count);
        if (bytes.size() == count) {
            return bytes;
        }
    }

    // /dev/urandom is always available on Sailfish OS. This is only a
    // last-resort fallback that should never be reached.
    QByteArray bytes(count, 0);
    for (int i = 0; i < count; ++i) {
        bytes[i] = static_cast<char>(qrand() & 0xff);
    }
    return bytes;
}

QString createPassword()
{
    static const char alphabet[] =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    const int alphabetSize = static_cast<int>(sizeof(alphabet)) - 1;
    const QByteArray bytes = randomBytes(32);

    QString password;
    password.reserve(bytes.size());
    for (int i = 0; i < bytes.size(); ++i) {
        const int index = static_cast<unsigned char>(bytes.at(i)) % alphabetSize;
        password.append(QLatin1Char(alphabet[index]));
    }
    return password;
}

quint16 createPort()
{
    const QByteArray bytes = randomBytes(2);
    const quint32 value = (static_cast<quint32>(
                               static_cast<unsigned char>(bytes.at(0)))
                           << 8)
        | static_cast<unsigned char>(bytes.at(1));
    return static_cast<quint16>(minPort + value % (maxPort - minPort + 1));
}

} // namespace

/// Serves one accepted TCP connection: key exchange, password authentication,
/// session channel and SFTP subsystem.
class SftpConnectionThread : public QThread
{
public:
    SftpConnectionThread(SshSession session, QString rootPath, QString user,
                         QString password)
        : m_session(std::move(session))
        , m_rootPath(std::move(rootPath))
        , m_user(std::move(user))
        , m_password(std::move(password))
    {
    }

    ~SftpConnectionThread() override
    {
        // SFTP sessions keep pointers to their channel, so they have to go
        // first.
        m_sftpSessions.clear();

        if (m_session.cPointer()) {
            ssh_disconnect(m_session.cPointer());
        }

        for (ssh_channel channel : m_channels) {
            ssh_channel_free(channel);
        }
    }

    void run() override;

private:
    static ssh_channel openSessionChannel(ssh_session session, void *userdata);
    static int authenticatePassword(ssh_session session, const char *user,
                                    const char *password, void *userdata);
    static int requestSubsystem(ssh_session session, ssh_channel channel,
                                const char *subsystem, void *userdata);
    static void closeChannel(ssh_session session, ssh_channel channel,
                             void *userdata);

    void startSftpSession(ssh_channel channel);

    SshSession m_session;
    QString m_rootPath;
    QString m_user;
    QString m_password;

    ssh_server_callbacks_struct m_serverCallbacks = {};
    ssh_channel_callbacks_struct m_channelCallbacks = {};

    std::vector<ssh_channel> m_channels;
    std::vector<std::unique_ptr<SftpServerSession>> m_sftpSessions;

    int m_remainingTries = maxPasswordTries;
    bool m_authenticated = false;
};

void SftpConnectionThread::run()
{
    ssh_session session = m_session.cPointer();
    if (!session) {
        return;
    }

    m_serverCallbacks.userdata = this;
    m_serverCallbacks.auth_password_function = &SftpConnectionThread::authenticatePassword;
    m_serverCallbacks.channel_open_request_session_function =
        &SftpConnectionThread::openSessionChannel;
    ssh_callbacks_init(&m_serverCallbacks);
    if (ssh_set_server_callbacks(session, &m_serverCallbacks) != SSH_OK) {
        qCWarning(logger) << "Cannot install SSH server callbacks:"
                          << ssh_get_error(session);
        return;
    }

    m_channelCallbacks.userdata = this;
    m_channelCallbacks.channel_subsystem_request_function =
        &SftpConnectionThread::requestSubsystem;
    m_channelCallbacks.channel_close_function = &SftpConnectionThread::closeChannel;
    m_channelCallbacks.channel_eof_function = &SftpConnectionThread::closeChannel;
    ssh_callbacks_init(&m_channelCallbacks);

    if (ssh_handle_key_exchange(session) != SSH_OK) {
        qCWarning(logger) << "SSH key exchange failed:" << ssh_get_error(session);
        return;
    }

    ssh_set_auth_methods(session, SSH_AUTH_METHOD_PASSWORD);

    ssh_event event = ssh_event_new();
    if (!event) {
        qCWarning(logger) << "Cannot allocate SSH event loop";
        return;
    }
    ssh_event_add_session(event, session);

    // Wait until the client authenticated and opened the SFTP subsystem.
    while (m_sftpSessions.empty() && !isInterruptionRequested()) {
        int rc = ssh_event_dopoll(event, 1000);
        if (rc == SSH_ERROR) {
            qCWarning(logger) << "SSH event loop failed:" << ssh_get_error(session);
            break;
        }
    }
    ssh_event_free(event);

    if (m_sftpSessions.empty()) {
        return;
    }

    for (auto &sftpSession : m_sftpSessions) {
        if (sftpSession->init() != 0) {
            qCWarning(logger) << "Cannot initialize SFTP session:"
                              << ssh_get_error(session);
            return;
        }
    }

    while (!isInterruptionRequested()) {
        bool alive = true;
        for (auto &sftpSession : m_sftpSessions) {
            alive = sftpSession->readMessage() && alive;
        }
        if (!alive) {
            break;
        }
    }
}

ssh_channel SftpConnectionThread::openSessionChannel(ssh_session session, void *userdata)
{
    auto *self = static_cast<SftpConnectionThread *>(userdata);
    ssh_channel channel = ssh_channel_new(session);
    if (!channel) {
        return nullptr;
    }
    if (ssh_set_channel_callbacks(channel, &self->m_channelCallbacks) != SSH_OK) {
        ssh_channel_free(channel);
        return nullptr;
    }
    self->m_channels.push_back(channel);
    return channel;
}

int SftpConnectionThread::authenticatePassword(ssh_session, const char *user,
                                               const char *password, void *userdata)
{
    auto *self = static_cast<SftpConnectionThread *>(userdata);

    const QByteArray expectedUser = self->m_user.toUtf8();
    const QByteArray expectedPassword = self->m_password.toUtf8();
    if (qstrcmp(user, expectedUser.constData()) == 0
        && qstrcmp(password, expectedPassword.constData()) == 0) {
        self->m_authenticated = true;
        return SSH_AUTH_SUCCESS;
    }

    self->m_remainingTries -= 1;
    if (self->m_remainingTries <= 0) {
        qCWarning(logger) << "Too many failed authentication attempts, disconnecting";
        return SSH_AUTH_DENIED;
    }
    return SSH_AUTH_DENIED;
}

int SftpConnectionThread::requestSubsystem(ssh_session, ssh_channel channel,
                                           const char *subsystem, void *userdata)
{
    if (qstrcmp(subsystem, "sftp") != 0) {
        qCWarning(logger) << "Unsupported SSH subsystem requested:" << subsystem;
        return SSH_ERROR;
    }

    auto *self = static_cast<SftpConnectionThread *>(userdata);
    self->startSftpSession(channel);
    return SSH_OK;
}

void SftpConnectionThread::closeChannel(ssh_session, ssh_channel channel,
                                        void *userdata)
{
    Q_UNUSED(channel)

    auto *self = static_cast<SftpConnectionThread *>(userdata);
    qCDebug(logger) << "SSH channel closed, session" << self;
    // The channel stays in m_channels and is freed with the thread: the SFTP
    // session keeps using the handle until sftp_get_client_message() reports
    // the disconnection.
}

void SftpConnectionThread::startSftpSession(ssh_channel channel)
{
    sftp_session sftp = sftp_server_new(m_session.cPointer(), channel);
    if (!sftp) {
        qCWarning(logger) << "Cannot create SFTP server session";
        return;
    }

    m_sftpSessions.emplace_back(std::make_unique<SftpServerSession>(sftp, m_rootPath));
}

/// Accepts incoming TCP connections and hands each one to a session thread.
class SftpServer::AcceptThread : public QThread
{
public:
    AcceptThread(SshBind bind, ssh_key hostKey, QString rootPath, QString user,
                 QString password)
        : m_bind(std::move(bind))
        , m_hostKey(hostKey)
        , m_rootPath(std::move(rootPath))
        , m_user(std::move(user))
        , m_password(std::move(password))
    {
    }

    ~AcceptThread() override
    {
        if (m_hostKey) {
            ssh_key_free(m_hostKey);
        }
    }

    void run() override
    {
        while (!isInterruptionRequested()) {
            SshSession session;
            if (m_bind.accept(session) != SSH_OK) {
                if (!isInterruptionRequested()) {
                    qCWarning(logger) << "Cannot accept SSH connection:"
                                      << m_bind.errorString();
                }
                break;
            }

            auto *connection = new SftpConnectionThread(
                std::move(session), m_rootPath, m_user, m_password);
            connect(connection, &QThread::finished, connection, &QObject::deleteLater);
            connection->start(QThread::LowPriority);
        }
    }

    void shutdown()
    {
        requestInterruption();

        int fd = m_bind.fileDescriptor();
        if (fd >= 0) {
            ::shutdown(fd, SHUT_RDWR);
        }
    }

private:
    SshBind m_bind;
    ssh_key m_hostKey = nullptr;
    QString m_rootPath;
    QString m_user;
    QString m_password;
};

SftpServer::SftpServer(QString rootPath, QObject *parent)
    : QObject(parent)
    , m_rootPath(std::move(rootPath))
{
}

SftpServer::~SftpServer()
{
    stop();
}

bool SftpServer::start()
{
    if (m_running) {
        return true;
    }

    m_error.clear();

    // libssh is reference counted, this is safe to call more than once. The
    // daemon is long lived, so ssh_finalize() is never called.
    ssh_init();

    const QString canonicalRoot = QDir(m_rootPath).canonicalPath();
    if (!canonicalRoot.isEmpty()) {
        m_rootPath = canonicalRoot;
    }

    m_credentials.user = QStringLiteral("kdeconnect");
    m_credentials.password = createPassword();
    m_credentials.port = 0;

    for (int attempt = 0; attempt < maxPortTries; ++attempt) {
        const quint16 port = createPort();

        SshBind bind;
        const QByteArray bindAddress = QByteArrayLiteral("0.0.0.0");
        const QByteArray portString = QByteArray::number(port);
        bind.setOption(SSH_BIND_OPTIONS_BINDADDR, bindAddress.constData());
        bind.setOption(SSH_BIND_OPTIONS_BINDPORT_STR, portString.constData());

        ssh_key hostKey = nullptr;
        if (ssh_pki_generate(SSH_KEYTYPE_ED25519, 0, &hostKey) != SSH_OK
            && ssh_pki_generate(SSH_KEYTYPE_RSA, 2048, &hostKey) != SSH_OK) {
            m_error = tr("Cannot generate an SSH host key");
            return false;
        }

        if (bind.setOption(SSH_BIND_OPTIONS_IMPORT_KEY, hostKey) != SSH_OK) {
            ssh_key_free(hostKey);
            m_error = tr("Cannot use the generated SSH host key: %1")
                          .arg(QString::fromUtf8(bind.errorString()));
            return false;
        }

        if (bind.listen() != SSH_OK) {
            // Probably a busy port: try another one.
            qCDebug(logger) << "Cannot listen on port" << port << ":"
                            << bind.errorString();
            ssh_key_free(hostKey);
            continue;
        }

        m_credentials.port = port;
        m_acceptThread = std::make_unique<AcceptThread>(
            std::move(bind), hostKey, m_rootPath, m_credentials.user,
            m_credentials.password);
        m_acceptThread->start();
        m_running = true;

        qCDebug(logger) << "SFTP server listening on port" << port
                        << "with root" << m_rootPath;
        return true;
    }

    m_error = tr("No free port available for the SFTP server");
    return false;
}

void SftpServer::stop()
{
    if (!m_running) {
        return;
    }

    m_running = false;

    if (m_acceptThread) {
        m_acceptThread->shutdown();
        m_acceptThread->wait(5000);
        m_acceptThread.reset();
    }

    qCDebug(logger) << "SFTP server stopped";
}
