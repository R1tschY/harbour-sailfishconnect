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

#include "sftpserverplugin.h"

#include <QDir>
#include <QHostAddress>
#include <QLoggingCategory>
#include <QNetworkInterface>

#include <KPluginFactory>

#include "sftpserver.h"

K_PLUGIN_CLASS_WITH_JSON(SftpServerPlugin, "sailfishconnect_sftpserver.json")

Q_LOGGING_CATEGORY(loggerPlugin, "kdeconnect.plugin.sftpserver")

static const QString PACKET_TYPE_SFTP = QStringLiteral("kdeconnect.sftp");
static const QString PACKET_TYPE_SFTP_REQUEST =
    QStringLiteral("kdeconnect.sftp.request");

SftpServerPlugin::SftpServerPlugin(QObject *parent, const QVariantList &args)
    : KdeConnectPlugin(parent, args)
{
}

SftpServerPlugin::~SftpServerPlugin()
{
    if (m_server) {
        m_server->stop();
    }
}

void SftpServerPlugin::receivePacket(const NetworkPacket &np)
{
    if (np.type() != PACKET_TYPE_SFTP_REQUEST) {
        return;
    }

    if (!np.get<bool>(QStringLiteral("startBrowsing"), true)) {
        stopServer(true);
        return;
    }

    if (!m_server) {
        m_server = std::make_unique<SftpServer>(exportedPath(), this);
    }

    if (!m_server->isRunning() && !m_server->start()) {
        qCWarning(loggerPlugin) << "Cannot start SFTP server:"
                                << m_server->errorString();
        sendErrorMessage(m_server->errorString());
        m_server.reset();
        return;
    }

    sendCredentialsPacket();
}

QString SftpServerPlugin::exportedPath() const
{
    return QDir::homePath();
}

QString SftpServerPlugin::localIpAddress() const
{
    const QHostAddress address = device()->getLocalIpAddress();
    if (!address.isNull()) {
        return address.toString();
    }

    // Fallback: first IPv4 address of an interface that is up.
    const auto interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface &interface : interfaces) {
        const auto flags = interface.flags();
        if (!(flags & QNetworkInterface::IsUp)
            || (flags & QNetworkInterface::IsLoopBack)) {
            continue;
        }
        for (const QNetworkAddressEntry &entry : interface.addressEntries()) {
            if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                return entry.ip().toString();
            }
        }
    }

    return QString();
}

void SftpServerPlugin::sendCredentialsPacket()
{
    const SftpServer::Credentials &credentials = m_server->credentials();

    NetworkPacket np(PACKET_TYPE_SFTP);
    np.set(QStringLiteral("ip"), localIpAddress());
    np.set(QStringLiteral("port"), QString::number(credentials.port));
    np.set(QStringLiteral("user"), credentials.user);
    np.set(QStringLiteral("password"), credentials.password);
    // The server exports its root as "/" in the SFTP namespace.
    np.set(QStringLiteral("path"), QStringLiteral("/"));
    // Let the desktop show useful shortcuts instead of its Android defaults
    // ("/DCIM/Camera" does not exist on Sailfish OS).
    np.set(QStringLiteral("multiPaths"),
           QStringList{QStringLiteral("/"), QStringLiteral("/Pictures/Camera")});
    np.set(QStringLiteral("pathNames"),
           QStringList{tr("All files"), tr("Camera pictures")});
    sendPacket(np);
}

void SftpServerPlugin::sendErrorMessage(const QString &message)
{
    NetworkPacket np(PACKET_TYPE_SFTP);
    np.set(QStringLiteral("errorMessage"), message);
    sendPacket(np);
}

void SftpServerPlugin::stopServer(bool notifyPeer)
{
    if (!m_server) {
        return;
    }

    const bool wasRunning = m_server->isRunning();
    m_server->stop();
    m_server.reset();

    if (wasRunning && notifyPeer) {
        NetworkPacket np(PACKET_TYPE_SFTP);
        np.set(QStringLiteral("serverRunning"), false);
        sendPacket(np);
    }
}

#include "sftpserverplugin.moc"
