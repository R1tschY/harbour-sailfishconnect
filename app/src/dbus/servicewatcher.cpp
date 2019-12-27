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

#include "servicewatcher.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QLoggingCategory>

namespace SailfishConnect {

static Q_LOGGING_CATEGORY(logger, "sailfishconnect.dbus.service-watcher")

    DBusServiceWatcher::DBusServiceWatcher(QObject* parent)
    : QObject(parent)
    , m_interface(QDBusConnection::sessionBus().interface())
{
    connect(m_interface, &QDBusConnectionInterface::serviceRegistered,
        this, &DBusServiceWatcher::onServiceRegistered);
    connect(m_interface, &QDBusConnectionInterface::serviceUnregistered,
        this, &DBusServiceWatcher::onServiceUnregistered);
}

void DBusServiceWatcher::setService(const QString& value)
{
    if (m_service == value)
        return;

    m_service = value;
    Q_EMIT serviceChanged();

    auto reply = m_interface->isServiceRegistered(m_service);
    if (reply.isValid()) {
        setRegistered(reply.value());
    } else {
        qCWarning(logger)
            << "Call to isServiceRegistered failed:"
            << reply.error();
        setRegistered(false);
    }
}

void DBusServiceWatcher::onServiceRegistered(const QString &service)
{
    if (service == m_service) {
        setRegistered(true);
    }
}

void DBusServiceWatcher::onServiceUnregistered(const QString &service)
{
    if (service == m_service) {
        setRegistered(false);
    }
}

} // namespace SailfishConnect