#include "kdeconnect.h"

#include <QLoggingCategory>

namespace SailfishConnect {

static Q_LOGGING_CATEGORY(logger, "sailfishconnect.dbus-api")

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

QString DaemonApi::announcedName() {
    auto reply = DaemonDbusInterface::announcedName();
    reply.waitForFinished();
    if (!reply.isValid()) {
        qCWarning(logger) 
            << "Getting announcedName failed" 
            << reply.error();
        return QString();
    } else {
        return reply.value();
    }
}

} // namespace SailfishConnect