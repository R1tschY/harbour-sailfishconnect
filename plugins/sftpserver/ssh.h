#include <libssh/callbacks.h>
#include <libssh/libssh.h>
#include <libssh/server.h>
#include <libssh/sftp.h>

#pragma once


#define _SSH_CONCAT_(a, b) a ## b
#define _SSH_CONCAT(a, b) _SSH_CONCAT_(a, b)
#define _SSH_UNIQUE_PTR(name, ty, free_fn) \
    struct _SSH_CONCAT(name, Destructor) { \
        using pointer = ty; \
        void operator()(ty ptr) { free_fn(ptr); } \
    }; \
    using name = std::unique_ptr<ty, _SSH_CONCAT(name, Destructor)>

_SSH_UNIQUE_PTR(SshSessionPtr, ssh_session, ssh_free);
_SSH_UNIQUE_PTR(SshChannelPtr, ssh_channel, ssh_channel_free);
_SSH_UNIQUE_PTR(SftpSessionPtr, sftp_session, sftp_free);
_SSH_UNIQUE_PTR(SshStringPtr, ssh_string, ssh_string_free);
_SSH_UNIQUE_PTR(SshCharsPtr, char*, ssh_string_free_char);
_SSH_UNIQUE_PTR(SshBindPtr, ssh_bind, ssh_bind_free);

#undef _SSH_UNIQUE_PTR
#undef _SSH_CONCAT
#undef _SSH_CONCAT_

template<typename T>
class SshResult {
private:
    SshResult();

public:
    
};


class SshSession {
public:
    SshSession()
    : ptr(ssh_new())
    { }

    const char* errorString() {
        return ssh_get_error(ptr.get());
    }

    int errorCode() {
        return ssh_get_error_code(ptr.get());
    }

    ssh_session cPointer() const {
        return ptr.get();
    }

private:
    SshSessionPtr ptr;
};

class SshBind {
public:
    SshBind()
    : ptr(ssh_bind_new())
    { }

    int setOption(ssh_bind_options_e type, const void *value) {
        return ssh_bind_options_set(ptr.get(), type, value);
    }

    int listen() {
        return ssh_bind_listen(ptr.get());
    }

    int accept(SshSession& session) {
        return ssh_bind_accept(ptr.get(), session.cPointer());
    }

    const char* errorString() {
        return ssh_get_error(ptr.get());
    }

    int errorCode() {
        return ssh_get_error_code(ptr.get());
    }

    ssh_bind cPointer() {
        return ptr.get();
    }

private:
    SshBindPtr ptr;
};

class SshContext {
public:
    SshContext() { ssh_init(); }
    ~SshContext() { ssh_finalize(); }
};

