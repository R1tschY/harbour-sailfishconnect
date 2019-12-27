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

#ifndef SAILFISHCONNECT_H
#define SAILFISHCONNECT_H

#include <QString>

namespace SailfishConnect {

/**
 * @brief dbus service name of Sailfish Connect
 */
extern QString DBUS_SERVICE_NAME;

/**
 * @brief package version
 */
extern QString PACKAGE_VERSION;

/**
 * @brief package name (normally "harbour-sailfishconnect")
 */
extern QString PACKAGE_NAME;

/**
 * @brief pretty package name (normally "Sailfish Connect")
 */
extern QString PRETTY_PACKAGE_NAME;

} // namespace SailfishConnect

#endif // SAILFISHCONNECT_H
