/*
 * Copyright 2019 Lieven Hey <t.schilling@snafu.de>.
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

#include "remoteinput.h"
#include <QtPlugin>
#include <QMap>
#include <sailfishconnect/device.h>

QMap<QString, int> specialKeysMap = {
    //0,              // Invalid
    {"backspace", 1},
    {"tab", 2},
    //XK_Linefeed,    // 3
    {"left", 4},
    {"up", 5},
    {"right", 6},
    {"down", 7},
    {"pgup", 8},
    {"pgdn", 9},
    {"home", 10},
    {"end", 11},
    {"enter", 12},
    {"del", 13},
    {"esc", 14},
    //{Qt::Key_SysReq, 15},
    //{Qt::Key_ScrollLock, 16},
    //0,              // 17
    //0,              // 18
    //0,              // 19
    //0,              // 20
    //{Qt::Key_F1, 21},
    //{Qt::Key_F2, 22},
    //{Qt::Key_F3, 23},
    //{Qt::Key_F4, 24},
    //{Qt::Key_F5, 25},
    //{Qt::Key_F6, 26},
    //{Qt::Key_F7, 27},
    //{Qt::Key_F8, 28},
    //{Qt::Key_F9, 29},
    //{Qt::Key_F10, 30},
    //{Qt::Key_F11, 31},
    //{Qt::Key_F12, 32},
};

SailfishConnect::RemoteInputPlugin::RemoteInputPlugin(Device *device,
        const QString &name, const QSet<QString> &outgoingCapabilities)
    : KdeConnectPlugin(device, name, outgoingCapabilities)
{
}

bool SailfishConnect::RemoteInputPlugin::receivePacket(const NetworkPacket &np)
{
    Q_UNUSED(np);
    return false;
}

void SailfishConnect::RemoteInputPlugin::sendKeyPress(const QString &key,
    bool shift, bool ctrl, bool alt) const
{
    NetworkPacket np("kdeconnect.mousepad.request", {{"key", key},
                        {"specialKey", specialKeysMap[key]}, {"shift", shift},
                        {"ctrl", ctrl}, {"alt", alt}, {"sendAck", true}});
    sendPacket(np);
}

QString SailfishConnect::RemoteInputPluginFactory::name() const
{
    return tr("Remote Keyboard");
}

QString SailfishConnect::RemoteInputPluginFactory::description() const
{
    return tr("Type on remote compute using a virtual keyboard.");
}

QString SailfishConnect::RemoteInputPluginFactory::iconUrl() const
{
    return QStringLiteral("image://theme/icon-m-keyboard");
}

Q_IMPORT_PLUGIN(RemoteInputPluginFactory)
