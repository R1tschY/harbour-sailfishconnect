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