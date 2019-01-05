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


#include "touchpadplugin.h"

#include <QPoint>
#include <QtPlugin>

#include <sailfishconnect/device.h>

namespace SailfishConnect {

TouchpadPlugin::TouchpadPlugin(
        Device* device,
        const QString &name,
        const QSet<QString> &outgoingCapabilities)
    : KdeConnectPlugin(device, name, outgoingCapabilities)
{ }

bool TouchpadPlugin::receivePacket(const NetworkPacket &)
{
    return false;
}

void TouchpadPlugin::move(int dx, int dy)
{
    NetworkPacket np("kdeconnect.mousepad.request", {
        {"dx", dx},
        {"dy", dy}
    });
    sendPacket(np);
}

void TouchpadPlugin::scroll(float dx, float dy)
{
    NetworkPacket np("kdeconnect.mousepad.request", {
        {"scroll", true},
        {"dx", dx},
        {"dy", dy}
    });
    sendPacket(np);
}

void TouchpadPlugin::sendCommand(const QString &name, bool val)
{
    NetworkPacket np("kdeconnect.mousepad.request", {{ name, val }});
    sendPacket(np);
}

QString TouchpadPluginFactory::name() const
{
    return tr("Touchpad");
}

QString TouchpadPluginFactory::description() const
{
    return tr("Control remote computer through mouse movements and clicks.");
}

QString TouchpadPluginFactory::iconUrl() const
{
    return QStringLiteral("image://theme/icon-m-gesture");
}

} // namespace SailfishConnect

Q_IMPORT_PLUGIN(TouchpadPluginFactory)
