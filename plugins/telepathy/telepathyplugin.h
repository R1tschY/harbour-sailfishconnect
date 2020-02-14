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


#ifndef TELEPATHYPLUGIN_H
#define TELEPATHYPLUGIN_H

#include <QDBusError>

#include <core/kdeconnectplugin.h>


class TelepathyPlugin : public KdeConnectPlugin
{
    Q_OBJECT
public:
    using KdeConnectPlugin::KdeConnectPlugin;

    void connected() override { }
    bool receivePacket(const NetworkPacket &np) override;

private slots:
    void startSmsSuccess();
    void startSmsError(const QDBusError& error);
};

#endif // TELEPATHYPLUGIN_H
