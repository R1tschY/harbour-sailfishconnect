#ifndef APPDAEMON_H
#define APPDAEMON_H

#include "core/daemon.h"

namespace SailfishConnect {

class AppDaemon : public Daemon
{
    Q_OBJECT
public:
    AppDaemon(QObject* parent = nullptr) : Daemon(parent) {}

    void askPairingConfirmation(Device* device) override;

    void reportError(const QString & title, const QString & description) override;

signals:
    void askForPairingConfirmation(Device* deviceId);
};

} // namespace SailfishConnect

#endif // APPDAEMON_H
