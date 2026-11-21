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

#pragma once

#include <libssh/callbacks.h>
#include <libssh/libssh.h>
#include <libssh/server.h>
#include <libssh/sftp.h>

#include <memory>

#define _SSH_CONCAT_(a, b) a##b
#define _SSH_CONCAT(a, b) _SSH_CONCAT_(a, b)
#define _SSH_UNIQUE_PTR(name, ty, free_fn)                                   \
    struct _SSH_CONCAT(name, Destructor) {                                   \
        using pointer = ty;                                                  \
        void operator()(ty ptr)                                              \
        {                                                                    \
            free_fn(ptr);                                                    \
        }                                                                    \
    };                                                                       \
    using name = std::unique_ptr<ty, _SSH_CONCAT(name, Destructor)>

_SSH_UNIQUE_PTR(SshSessionPtr, ssh_session, ssh_free);
_SSH_UNIQUE_PTR(SshChannelPtr, ssh_channel, ssh_channel_free);
_SSH_UNIQUE_PTR(SftpSessionPtr, sftp_session, sftp_free);
_SSH_UNIQUE_PTR(SshStringPtr, ssh_string, ssh_string_free);
_SSH_UNIQUE_PTR(SshBindPtr, ssh_bind, ssh_bind_free);

#undef _SSH_UNIQUE_PTR
#undef _SSH_CONCAT
#undef _SSH_CONCAT_

/// Lightweight owner for an ssh_session.
///
/// Sessions handed out by ssh_bind_accept() are owned by the caller and can
/// be transferred between threads with release()/adopt().
class SshSession
{
public:
    SshSession()
        : ptr(ssh_new())
    {
    }

    explicit SshSession(ssh_session session)
        : ptr(session)
    {
    }

    SshSession(SshSession &&other) noexcept = default;
    SshSession &operator=(SshSession &&other) noexcept = default;
    SshSession(const SshSession &) = delete;
    SshSession &operator=(const SshSession &) = delete;

    ssh_session release()
    {
        return ptr.release();
    }

    const char *errorString()
    {
        return ssh_get_error(ptr.get());
    }

    int errorCode()
    {
        return ssh_get_error_code(ptr.get());
    }

    ssh_session cPointer() const
    {
        return ptr.get();
    }

private:
    SshSessionPtr ptr;
};

/// Lightweight owner for an ssh_bind.
class SshBind
{
public:
    SshBind()
        : ptr(ssh_bind_new())
    {
    }

    int setOption(ssh_bind_options_e type, const void *value)
    {
        return ssh_bind_options_set(ptr.get(), type, value);
    }

    int listen()
    {
        return ssh_bind_listen(ptr.get());
    }

    int accept(SshSession &session)
    {
        return ssh_bind_accept(ptr.get(), session.cPointer());
    }

    int fileDescriptor() const
    {
        return ssh_bind_get_fd(ptr.get());
    }

    const char *errorString()
    {
        return ssh_get_error(ptr.get());
    }

    int errorCode()
    {
        return ssh_get_error_code(ptr.get());
    }

    ssh_bind cPointer() const
    {
        return ptr.get();
    }

private:
    SshBindPtr ptr;
};
