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

#include "sendnotificationsplugin.h"

namespace SailfishConnect {

SendNotificationsPlugin::SendNotificationsPlugin(
        Device* device,
        const QString &name,
        const QSet<QString> &outgoingCapabilities)
    : KdeConnectPlugin(device, name, outgoingCapabilities),
      m_listener(new NotificationsListener(this))
{ }

bool SendNotificationsPlugin::receivePackage(const NetworkPackage&)
{
    return false;
}

QString SendNotificationsPluginFactory::name() const
{
    return tr("Send notifcations");
}

QString SendNotificationsPluginFactory::description() const
{
    return tr("Send notifications to the remote computer.");
}

QString SendNotificationsPluginFactory::iconUrl() const
{
    return "image://theme/icon-m-notifications";
}

} // namespace SailfishConnect

Q_IMPORT_PLUGIN(SendNotificationsPluginFactory)
