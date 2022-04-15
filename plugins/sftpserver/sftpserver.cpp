#include <argp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <optional>
#include <vector>

#include <QThread>
#include <QDebug>
#include <libssh/callbacks.h>
#include <libssh/libssh.h>
#include <libssh/server.h>
#include <libssh/sftp.h>

#include <sftpservercommands.h>

#define KEYS_FOLDER "/home/richard/tmp/"

#define USER "myuser"
#define PASSWORD "mypassword"

class SshServer {
public:
    SshServer(const QString& user, const QString& password, int passwordTries)
        : m_user(user)
        , m_password(password)
        , m_passwordTries(passwordTries)
    {
    }

    int passwordTries() const
    {
        return m_passwordTries;
    }

    bool checkPassword(const char* user, const char* password) const
    {
        return strcmp(user, USER) == 0 && strcmp(password, PASSWORD) == 0;
    }

private:
    QString m_user;
    QString m_password;
    int m_passwordTries;
};

class SftpServerChannel {
public:
    SftpServerChannel(SshChannelPtr channel)
        : m_channel(std::move(channel))
        , m_sftpSession()
    {
    }

    void createSftpSession(SshSession& session)
    {
        Q_ASSERT(!m_sftpSession.has_value());
        printf("createSftpSession");
        m_sftpSession.emplace(sftp_server_new(session.cPointer(), m_channel.get()));
        Q_ASSERT(m_sftpSession.has_value());
    }

    ssh_channel getChannelPtr() const { return m_channel.get(); }
    bool hasSftpSession() const { return m_sftpSession.has_value(); }

    int initSftpSession()
    {
        Q_ASSERT(m_sftpSession.has_value());
        m_inited = true;
        return m_sftpSession.value().init();
    }

    bool readMessage()
    {
        if (!m_inited)
            initSftpSession();
        return m_sftpSession.value().readMessage();
    }

private:
    SshChannelPtr m_channel;
    std::optional<SftpServerSession> m_sftpSession;
    bool m_inited = false;
};

class SshServerSession {
public:
    SshServerSession(SshSession session, SshServer* server);

    ~SshServerSession()
    {
        ssh_disconnect(m_session.cPointer());
    }

    void disconnect()
    {
        ssh_disconnect(m_session.cPointer());
        m_connected = false;
    }

    bool connected()
    {
        return m_connected;
    }

    ssh_session sshSession() const
    {
        return m_session.cPointer();
    }

    bool hasSftpChannel() const
    {
        return m_channels.size() > 0 && m_channels.front().hasSftpSession();
    }

    int getChannelIndex(ssh_channel channel) const;

    int authorizeWithPassword(const char* user, const char* password);
    void startSftpSession(ssh_channel channel);
    ssh_channel createChannel();
    void closeChannel(ssh_channel channel);

private:
    friend class SshFsThread;

    SshSession m_session;
    ssh_server_callbacks_struct m_cb = {};
    ssh_callbacks_struct m_callbacks = {};

    std::vector<SftpServerChannel> m_channels;
    ssh_channel_callbacks_struct m_channel_cb = {};

    SshServer* m_server = nullptr;

    int m_leftTries = 3;
    bool m_connected = true;
    bool m_authenticated = false;
};

class SshFsThread : public QThread {
public:
    SshFsThread(SshSession session, SshServer* server, QObject* parent = nullptr)
        : QThread(parent)
        , m_session(std::move(session), std::move(server))
    {
    }

    void run() override;

private:
    SshServerSession m_session;
};

void SshFsThread::run()
{
    printf("ssh_handle_key_exchange\n");
    if (ssh_handle_key_exchange(m_session.sshSession())) {
        printf("ssh_handle_key_exchange: %s\n", ssh_get_error(m_session.sshSession()));
        return;
    }
    printf("ssh_set_auth_methods\n");
    ssh_set_auth_methods(m_session.sshSession(), SSH_AUTH_METHOD_PASSWORD);
    ssh_event mainloop = ssh_event_new();
    printf("ssh_event_add_session\n");
    ssh_event_add_session(mainloop, m_session.sshSession());

    while (!m_session.hasSftpChannel()) {
        printf("POLL\n");
        int r = ssh_event_dopoll(mainloop, -1);
        if (r == SSH_ERROR) {
            printf("Error : %s\n", ssh_get_error(m_session.sshSession()));
            return;
        }
        if (isInterruptionRequested()) {
            printf("Thread interrupted\n");
            return;
        }
        if (!m_session.connected()) {
            printf("Error, exiting loop\n");
            return;
        }
    }

    printf("Got everything to start: %ld\n", m_session.m_channels.size());
    for (auto& channel : m_session.m_channels) {
        int res = channel.initSftpSession();
        if (res < 0) {
            printf("Error sftp_server_init\n");
            return;
        }
    }

    std::vector<ssh_channel> read_channels;
    std::vector<ssh_channel> except_channels;
    timeval timeout = {
        .tv_sec = 5,
        .tv_usec = 0,
    };
    sftp_client_message msg;
    while (!isInterruptionRequested()) {
        for (auto& channel : m_session.m_channels) {
            bool res = channel.readMessage();
            if (!res)
                return;
        }

        // read_channels.clear();
        // except_channels.clear();
        // for (auto& channel : m_session.m_channels) {
        //     read_channels.push_back(channel.getChannelPtr());
        //     except_channels.push_back(channel.getChannelPtr());
        // }
        // read_channels.push_back(nullptr);
        // except_channels.push_back(nullptr);

        // int res = ssh_channel_select(
        //     read_channels.data(), nullptr, except_channels.data(), &timeout);
        // printf("select\n");
        // if (res == SSH_OK) {
        //     for (auto iter = read_channels.data(); *iter != nullptr; ++iter) {
        //         printf("read channel %p\n", *iter);
        //         int index = m_session.getChannelIndex(*iter);
        //         if (index < 0) continue;
        //         m_session.m_channels[index].readMessage();
        //     }
        //     for (auto iter = except_channels.data(); *iter != nullptr; ++iter) {
        //         int index = m_session.getChannelIndex(*iter);
        //         if (index < 0) continue;
        //         m_session.m_channels[index].readMessage();
        //     }
        // } else if (res == SSH_EINTR) {
        //     printf("SSH_EINTR");
        //     continue;
        // } else {
        //     printf("Error ssh_channel_select: %s", m_session.m_session.errorString());
        //     return;
        // }
    }
}

static ssh_channel new_session_channel(ssh_session session, void* userdata)
{
    Q_UNUSED(session);
    Q_ASSERT(userdata != nullptr);
    return reinterpret_cast<SshServerSession*>(userdata)->createChannel();
}

ssh_channel SshServerSession::createChannel()
{
    printf("Allocated session channel\n");
    ssh_channel channel = ssh_channel_new(m_session.cPointer());
    m_channels.emplace_back(SshChannelPtr(channel));
    ssh_set_channel_callbacks(channel, &m_channel_cb);
    return channel;
}

static int auth_password(ssh_session session, const char* user,
    const char* password, void* userdata)
{
    Q_ASSERT(userdata != nullptr);
    return reinterpret_cast<SshServerSession*>(userdata)->authorizeWithPassword(user, password);
}

int SshServerSession::authorizeWithPassword(const char* user, const char* password)
{
    printf("Authenticating user %s pwd %s\n", user, password);
    if (m_server->checkPassword(user, password)) {
        m_authenticated = true;
        printf("Authenticated\n");
        return SSH_AUTH_SUCCESS;
    }
    m_leftTries -= 1;
    if (m_leftTries == 0) {
        printf("Too many authentication tries\n");
        disconnect();
        return SSH_AUTH_DENIED;
    }
    return SSH_AUTH_DENIED;
}

static int subsystem_request(
    ssh_session session,
    ssh_channel channel,
    const char* subsystem,
    void* userdata)
{
    printf("Subsystem request %s\n", subsystem);

    if (strcmp(subsystem, "sftp") == 0) {
        reinterpret_cast<SshServerSession*>(userdata)->startSftpSession(channel);
        printf("Start SFTP\n");
        return SSH_OK;
    }

    return SSH_OK;
}

static void close_channel(
    ssh_session session,
    ssh_channel channel,
    void* userdata)
{
    auto* self = reinterpret_cast<SshServerSession*>(userdata);
    Q_ASSERT(self != nullptr);
    self->closeChannel(channel);
}

static void logSession(ssh_session session, int priority,
    const char* message, void* userdata)
{
    qDebug() << priority << message;
    printf("%d %s", priority, message);
}

SshServerSession::SshServerSession(SshSession session, SshServer* server)
    : m_session(std::move(session))
    , m_server(server)
    , m_leftTries(server->passwordTries())
{
    printf("new_session_channel\n");

    m_cb.userdata = this;
    m_cb.auth_password_function = auth_password;
    m_cb.channel_open_request_session_function = new_session_channel;
    ssh_callbacks_init(&m_cb);
    ssh_set_server_callbacks(m_session.cPointer(), &m_cb);

    m_callbacks.userdata = this;
    m_callbacks.log_function = logSession;
    ssh_callbacks_init(&m_callbacks);

    m_channel_cb.userdata = this;
    m_channel_cb.channel_subsystem_request_function = subsystem_request;
    m_channel_cb.channel_close_function = close_channel;
    m_channel_cb.channel_eof_function = close_channel;
    ssh_callbacks_init(&m_channel_cb);

    bool config = false;
    ssh_options_set(m_session.cPointer(), SSH_OPTIONS_COMPRESSION, "yes");
    ssh_options_set(m_session.cPointer(), SSH_OPTIONS_PROCESS_CONFIG, &config);
    ssh_options_set(m_session.cPointer(), SSH_OPTIONS_LOG_VERBOSITY_STR, "4");
}

void SshServerSession::closeChannel(ssh_channel channel)
{
    printf("Channel closed by remote\n");
    int index = getChannelIndex(channel);
    Q_ASSERT(index >= 0);
    m_channels.erase(m_channels.begin() + index);
}

void SshServerSession::startSftpSession(ssh_channel channel)
{
    int index = getChannelIndex(channel);
    Q_ASSERT(index >= 0);

    m_channels[index].createSftpSession(m_session);
}

int SshServerSession::getChannelIndex(ssh_channel channel) const
{
    auto iter = std::find_if(
        m_channels.begin(), m_channels.end(),
        [=](auto& ch) { return ch.getChannelPtr() == channel; });
    if (iter != m_channels.end()) {
        return static_cast<int>(std::distance(m_channels.begin(), iter));
    }
    return -1;
}

const char* argp_program_version = "libssh server example " SSH_STRINGIFY(LIBSSH_VERSION);
const char* argp_program_bug_address = "<libssh@libssh.org>";

/* Program documentation. */
static char doc[] = "libssh -- a Secure Shell protocol implementation";

/* A description of the arguments we accept. */
static char args_doc[] = "BINDADDR";

/* The options we understand. */
static struct argp_option options[] = {
    { .name = "port",
        .key = 'p',
        .arg = "PORT",
        .flags = 0,
        .doc = "Set the port to bind",
        .group = 0 },
    { .name = "hostkey",
        .key = 'k',
        .arg = "FILE",
        .flags = 0,
        .doc = "Set the host key",
        .group = 0 },
    { .name = "dsakey",
        .key = 'd',
        .arg = "FILE",
        .flags = 0,
        .doc = "Set the dsa key",
        .group = 0 },
    { .name = "rsakey",
        .key = 'r',
        .arg = "FILE",
        .flags = 0,
        .doc = "Set the rsa key",
        .group = 0 },
    { .name = "verbose",
        .key = 'v',
        .arg = nullptr,
        .flags = 0,
        .doc = "Get verbose output",
        .group = 0 },
    { nullptr, 0, nullptr, 0, nullptr, 0 }
};

/* Parse a single option. */
static error_t parse_opt(int key, char* arg, struct argp_state* state)
{
    /* Get the input argument from argp_parse, which we
     * know is a pointer to our arguments structure.
     */
    ssh_bind sshbind = reinterpret_cast<ssh_bind>(state->input);

    switch (key) {
    case 'p':
        ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_BINDPORT_STR, arg);
        break;
    case 'd':
        ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_DSAKEY, arg);
        break;
    case 'k':
        ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_HOSTKEY, arg);
        break;
    case 'r':
        ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_RSAKEY, arg);
        break;
    case 'v':
        ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_LOG_VERBOSITY_STR, "3");
        break;
    case ARGP_KEY_ARG:
        if (state->arg_num >= 1) {
            /* Too many arguments. */
            argp_usage(state);
        }
        ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_BINDADDR, arg);
        break;
    case ARGP_KEY_END:
        if (state->arg_num < 1) {
            /* Not enough arguments. */
            argp_usage(state);
        }
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

/* Our argp parser. */
static struct argp argp = { options, parse_opt, args_doc, doc, nullptr, nullptr, nullptr };

class SshEventQueue {
public:
    SshEventQueue()
        : m_eventqueue(ssh_event_new())
    {
    }
    ~SshEventQueue() { ssh_event_free(m_eventqueue); }

    bool run(int timeout)
    {
        int rc = SSH_OK;
        while (rc == SSH_OK) {
            printf("POLL\n");
            rc = ssh_event_dopoll(m_eventqueue, timeout);
        }
        return rc != SSH_ERROR;
    }

private:
    ssh_event m_eventqueue;
    QString m_error;
};

int main(int argc, char** argv)
{
    printf("Debug: %d\n", __LINE__);
    SshServer server(USER, PASSWORD, 3);
    SshContext context;
    printf("Debug: %d\n", __LINE__);
    SshBind bind;
    printf("Debug: %d\n", __LINE__);
    //ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_ECDSAKEY, KEYS_FOLDER "ssh_host_ecdsa_key");
    bind.setOption(SSH_BIND_OPTIONS_RSAKEY, KEYS_FOLDER "ssh_host_rsa_key");
    int verbosity = SSH_LOG_TRACE;
    bind.setOption(SSH_BIND_OPTIONS_LOG_VERBOSITY, &verbosity);
    bool processConfig = false;
    bind.setOption(SSH_BIND_OPTIONS_PROCESS_CONFIG, &processConfig);
    // TODO: more options
    printf("Debug: %d\n", __LINE__);
    /*
     * Parse our arguments; every option seen by parse_opt will
     * be reflected in arguments.
     */
    argp_parse(&argp, argc, argv, 0, nullptr, bind.cPointer());
    printf("Debug: %d\n", __LINE__);
    if (bind.listen() < 0) {
        printf("Error listening to socket: %s\n", bind.errorString());
        return 1;
    }
    printf("Debug: %d\n", __LINE__);
    {
        std::vector<std::unique_ptr<SshFsThread>> clients;

        int r;
        while (true) {
            SshSession session;
            r = bind.accept(session);
            if (r == SSH_ERROR) {
                printf("error accepting a connection : %s\n", bind.errorString());
                break;
            }

            clients.emplace_back(std::make_unique<SshFsThread>(std::move(session), &server));
            clients.back()->start(QThread::LowPriority);
        }
    }

    return 0;
}
