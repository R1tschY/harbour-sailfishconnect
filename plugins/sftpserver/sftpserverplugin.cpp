/*
 * Copyright 2021 Richard Liebscher <richard.liebscher@gmail.com>.
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

#include <QLoggingCategory>
#include <KPluginFactory>


K_PLUGIN_CLASS_WITH_JSON(SftpServerPlugin, "sailfishconnect_sftpserver.json")

static Q_LOGGING_CATEGORY(logger, "kdeconnect.plugin.sftpserver");

static const QString PACKET_TYPE_SFTP = QStringLiteral("kdeconnect.sftp");

static const QString PACKET_TYPE_SFTP_REQUEST = QStringLiteral("kdeconnect.sftp.request");


void SftpServerPlugin::receivePacket(const NetworkPacket& np)
{
    if (np.type() == PACKET_TYPE_SFTP_REQUEST) {
        if (np.get<bool>(QStringLiteral("startBrowsing"))) {
            
        }

        NetworkPacket resultNp(PACKET_TYPE_SFTP);

        sendPacket(resultNp);
    }
}

#include "sftpserverplugin.moc"
