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

#include "remotekeyboardplugin.h"
#include <QHash>
#include <KPluginFactory>
#include <core/device.h>

K_PLUGIN_CLASS_WITH_JSON(RemoteKeyboardPlugin, "sailfishconnect_remotekeyboard.json")

QHash<QString, int> specialKeysMap = {
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

RemoteKeyboardPlugin::RemoteKeyboardPlugin(QObject* parent, const QVariantList& args)
    : KdeConnectPlugin(parent, args)
{
}

bool RemoteKeyboardPlugin::receivePacket(const NetworkPacket &np)
{
    // TODO: kdeconnect.mousepad.echo
    // TODO: kdeconnect.mousepad.keyboardstate
    Q_UNUSED(np);
    return true;
}

void RemoteKeyboardPlugin::sendKeyPress(const QString &key,
    bool shift, bool ctrl, bool alt) const
{
    NetworkPacket np("kdeconnect.mousepad.request", {
        {QStringLiteral("key"), key},
        {QStringLiteral("specialKey"), specialKeysMap.value(key, 0)},
        {QStringLiteral("shift"), shift},
        {QStringLiteral("ctrl"), ctrl},
        {QStringLiteral("alt"), alt}
    });
    sendPacket(np);
}

#include "remotekeyboardplugin.moc"