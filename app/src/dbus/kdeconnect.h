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

    Q_SCRIPTABLE QString encryptionInfo();
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