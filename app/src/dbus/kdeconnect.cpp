/*
 * Copyright 2019 Richard Liebscher <richard.liebscher@gmail.com>.
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

#include "kdeconnect.h"

#include <QLoggingCategory>

namespace SailfishConnect {

static Q_LOGGING_CATEGORY(logger, "sailfishconnect.dbus-api")

DeviceApi::DeviceApi(const QString& deviceId, QObject* parent)
: DeviceDbusInterface(deviceId, parent)
{
    connect(this, &DeviceApi::pluginsChanged, this, [this] { 
        m_loadedPluginsLoaded = false; 
        Q_EMIT pluginsChangedProxy();
    });
}

QStringList DeviceApi::loadedPlugins() {
    if (!m_loadedPluginsLoaded) {
        m_loadedPlugins = DeviceDbusInterface::loadedPlugins();
        m_loadedPluginsLoaded = true;
    }

    return m_loadedPlugins;
}

void checkForDbusError(const QDBusPendingCall& async) {
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async);
    QObject::connect(
        watcher, &QDBusPendingCallWatcher::finished,
        [](QDBusPendingCallWatcher *watcher) {
            if (watcher->isError()) {
                qCWarning(logger) 
                    << "Internal DBus call failed" 
                    << watcher->error();
            }
            watcher->deleteLater();
        });
}

} // namespace SailfishConnect