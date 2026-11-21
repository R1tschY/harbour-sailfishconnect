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

#include <core/kdeconnectplugin.h>

#include <memory>

class SftpServer;

/// Serves the user's home directory over SFTP to a paired KDE Connect peer.
///
/// The peer asks for a server by sending a kdeconnect.sftp.request packet.
/// We answer with the credentials of an embedded SSH/SFTP server, which the
/// peer then mounts with sshfs.
class SftpServerPlugin : public KdeConnectPlugin
{
    Q_OBJECT

public:
    SftpServerPlugin(QObject *parent, const QVariantList &args);
    ~SftpServerPlugin() override;

    void connected() override
    {
    }
    void receivePacket(const NetworkPacket &np) override;

private:
    void startServer();
    void stopServer(bool notifyPeer);
    void sendCredentialsPacket();
    void sendErrorMessage(const QString &message);
    QString exportedPath() const;
    QString localIpAddress() const;

    std::unique_ptr<SftpServer> m_server;
};
