/*
 * Copyright 2018 Richard Liebscher <richard.liebscher@gmail.com>.
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

#include "pingplugin.h"

#include <QLoggingCategory>

#include <core/networkpackage.h>

namespace SailfishConnect {

static Q_LOGGING_CATEGORY(logger, "SailfishConnect.Ping")

PingPlugin::PingPlugin(
        Device *device, const QString& name,
        const QSet<QString>& outgoingCapabilities)
    : KdeConnectPlugin(device, name, outgoingCapabilities)
{
    // TODO: notification_.setAppIcon();
    notification_.setAppName("Sailfish-Connect");
    notification_.setBody("Ping!");
    notification_.setCategory("device");
    notification_.setUrgency(Notification::Low);

    connect(&notification_, &Notification::clicked,
            this, &PingPlugin::resetCount);
    connect(&notification_, &Notification::closed,
            this, &PingPlugin::resetCount);
}

bool PingPlugin::receivePackage(const NetworkPackage& np)
{
    QString message =
            np.get<QString>(QStringLiteral("message"), QStringLiteral("Ping!"));

    qCInfo(logger) << "Ping:" << message << "from" << device()->name();

    count_ += 1;
    notification_.setSummary(device()->name());
    notification_.setBody(message);
    notification_.setItemCount(count_);
    notification_.publish();

    // TODO: remoteAction: openDevice(device()->id())

    return true;
}

void PingPlugin::resetCount()
{
    count_ = 0;
}

QString PingPluginFactory::name() const
{
    return tr("Ping");
}

QString PingPluginFactory::description() const
{
    return tr("Ping other device and receive pings.");
}

QString PingPluginFactory::iconUrl() const
{
    return "image://theme/icon-m-bubble-universal";
}

} // namespace SailfishConnect

Q_IMPORT_PLUGIN(PingPluginFactory)
