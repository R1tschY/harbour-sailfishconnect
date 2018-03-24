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

#ifndef APPDAEMON_H
#define APPDAEMON_H

#include "core/daemon.h"

#include <notification.h>

namespace SailfishConnect {

class AppDaemon : public Daemon
{
    Q_OBJECT
public:
    AppDaemon(QObject* parent = nullptr);

    void askPairingConfirmation(Device* device) override;

    void reportError(const QString & title, const QString & description) override;

signals:
    void askForPairingConfirmation(Device* deviceId);

private:
    Notification notification_;
};

} // namespace SailfishConnect

#endif // APPDAEMON_H
