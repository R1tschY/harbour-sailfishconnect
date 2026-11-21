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

#include <QObject>
#include <QString>

#include <atomic>
#include <memory>

#include <libssh/libssh.h>

/// Embedded SFTP server for a paired KDE Connect peer.
///
/// The server listens on a random TCP port of all interfaces, uses a freshly
/// generated SSH host key and accepts one user protected by a random password.
/// Those credentials are handed to the peer through the encrypted KDE Connect
/// link (kdeconnect.sftp packet); the peer then mounts the exported directory
/// with sshfs.
///
/// The exported directory is the user's home directory. Every path is
/// canonicalized and confined to that directory, so a peer can neither escape
/// the export through relative paths nor through symbolic links.
class SftpServer : public QObject
{
    Q_OBJECT

public:
    struct Credentials
    {
        QString user;
        QString password;
        quint16 port = 0;
    };

    explicit SftpServer(QString rootPath, QObject *parent = nullptr);
    ~SftpServer() override;

    /// Bind a free port and start accepting clients. Returns false on error
    /// with a human readable message in errorString().
    bool start();

    /// Stop accepting clients. Established connections are left alone.
    void stop();

    bool isRunning() const
    {
        return m_running;
    }

    const Credentials &credentials() const
    {
        return m_credentials;
    }

    QString rootPath() const
    {
        return m_rootPath;
    }

    QString errorString() const
    {
        return m_error;
    }

private:
    class AcceptThread;

    friend class AcceptThread;

    QString m_rootPath;
    Credentials m_credentials;
    QString m_error;
    std::unique_ptr<AcceptThread> m_acceptThread;
    std::atomic_bool m_running{false};
};
