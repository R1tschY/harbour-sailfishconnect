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

#include "remotekeyboard.h"
#include <QtPlugin>
#include <QMap>
#include <sailfishconnect/device.h>
#include <QDebug>

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
    {"F1", 21},
    {"F2", 22},
    {"F3", 23},
    {"F4", 24},
    {"F5", 25},
    {"F6", 26},
    {"F7", 27},
    {"F8", 28},
    {"F9", 29},
    {"F10", 30},
    {"F11", 31},
    {"F12", 32}
};

SailfishConnect::RemoteKeyboardPlugin::RemoteKeyboardPlugin(Device *device,
        const QString &name, const QSet<QString> &outgoingCapabilities)
    : KdeConnectPlugin(device, name, outgoingCapabilities)
{
}

bool SailfishConnect::RemoteKeyboardPlugin::receivePacket(const NetworkPacket &np)
{
    Q_UNUSED(np);
    return true;
}

void SailfishConnect::RemoteKeyboardPlugin::sendKeyPress(const QString &key,
    bool shift, bool ctrl, bool alt) const
{
    NetworkPacket np("kdeconnect.mousepad.request", {{"key", key},
                        {"specialKey", specialKeysMap.value(key, 0)},
                        {"shift", shift}, {"ctrl", ctrl}, {"alt", alt}});
    sendPacket(np);
}

QString SailfishConnect::RemoteKeyboardPluginFactory::name() const
{
    return tr("Remote Keyboard");
}

QString SailfishConnect::RemoteKeyboardPluginFactory::description() const
{
    return tr("Type on remote compute using a virtual keyboard.");
}

QString SailfishConnect::RemoteKeyboardPluginFactory::iconUrl() const
{
    return QStringLiteral("image://theme/icon-m-keyboard");
}

Q_IMPORT_PLUGIN(RemoteKeyboardPluginFactory)
