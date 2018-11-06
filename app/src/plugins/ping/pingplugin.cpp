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
#include <QCoreApplication>

#include <sailfishconnect/networkpackage.h>
#include <ui.h>
#include <sailfishconnect.h>

namespace SailfishConnect {

static Q_LOGGING_CATEGORY(logger, "SailfishConnect.Ping")

PingPlugin::PingPlugin(
        Device *device, const QString& name,
        const QSet<QString>& outgoingCapabilities)
    : KdeConnectPlugin(device, name, outgoingCapabilities)
{ }

bool PingPlugin::receivePackage(const NetworkPackage& np)
{
    QString message =
            np.get<QString>(QStringLiteral("message"), QStringLiteral("Ping!"));

    QString deviceName = device()->name();
    QString deviceId = device()->id();
    qCInfo(logger) << "Ping:" << message << "from" << deviceName;


    Notification *notification = new Notification(this);
    notification->setSummary(deviceName);
    notification->setPreviewSummary(deviceName);
    notification->setBody(message);
    notification->setPreviewBody(message);
    notification->setRemoteActions({ UI::openDevicePageDbusAction(deviceId) });

    connect(notification, &Notification::closed,
            this, [=](uint reason) { notification->deleteLater(); });

    notification->publish();

    return true;
}

void PingPlugin::sendPing()
{
    NetworkPackage np(QStringLiteral("kdeconnect.ping"));
    bool success = sendPackage(np);
    qCDebug(logger) << "sendPing:" << success;
}

void PingPlugin::sendPing(const QString& message)
{
    NetworkPackage np(QStringLiteral("kdeconnect.ping"));
    if (!message.isEmpty()) {
        np.set(QStringLiteral("message"), message);
    }
    bool success = sendPackage(np);
    qCDebug(logger).nospace() << "sendPing(" << message << "): " << success;
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
