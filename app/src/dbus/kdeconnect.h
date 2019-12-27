#include <dbusinterfaces.h>

namespace SailfishConnect {

void checkForDbusError(const QDBusPendingCall& async);


class DeviceApi : public DeviceDbusInterface {
    Q_OBJECT
public:
    using DeviceDbusInterface::DeviceDbusInterface;

    Q_SCRIPTABLE void unpair() {
        checkForDbusError(DeviceDbusInterface::unpair());
    }

    Q_SCRIPTABLE void requestPair() {
        checkForDbusError(DeviceDbusInterface::requestPair());
    }

    Q_SCRIPTABLE void acceptPairing() {
        checkForDbusError(DeviceDbusInterface::acceptPairing());
    }

    Q_SCRIPTABLE void rejectPairing() {
        checkForDbusError(DeviceDbusInterface::rejectPairing());
    }
};

class DaemonApi : public DaemonDbusInterface {
    Q_OBJECT
public:
    using DaemonDbusInterface::DaemonDbusInterface;

    Q_SCRIPTABLE QString announcedName();

    Q_SCRIPTABLE void setAnnouncedName(const QString& value) {
        checkForDbusError(DaemonDbusInterface::setAnnouncedName(value));
    }

    Q_SCRIPTABLE DeviceApi* getDevice(const QString& id)
    {
        return new DeviceApi(id);
    }
};

} // namespace SailfishConnect