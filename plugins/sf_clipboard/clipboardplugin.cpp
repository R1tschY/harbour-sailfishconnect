/*
 * Copyright 2020 Richard Liebscher <richard.liebscher@gmail.com>.
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

#include "clipboardplugin.h"

#include <KPluginFactory>

#include <QGuiApplication>
#include <QClipboard>
#include <core/networkpacket.h>

K_PLUGIN_CLASS_WITH_JSON(ClipboardPlugin, "sailfishconnect_clipboard.json")

static QString packetType = QStringLiteral("kdeconnect.clipboard");

ClipboardPlugin::ClipboardPlugin(QObject* parent, const QVariantList& args)
    : KdeConnectPlugin(parent, args)
    , m_clipboard(QGuiApplication::clipboard())
{}

bool ClipboardPlugin::receivePacket(const NetworkPacket &np)
{
    QString content = np.get<QString>(QStringLiteral("content"));
    m_clipboard->setText(content);
    return true;
}

void ClipboardPlugin::pushClipboard()
{
    QString clipboardText = m_clipboard->text();
    NetworkPacket np(packetType, {{"content", clipboardText}});
    sendPacket(np);
}

QString ClipboardPlugin::dbusPath() const
{
    return QStringLiteral("/modules/kdeconnect/devices/%1/sailfishconnect_clipboard")
        .arg(device()->id());
}

#include "clipboardplugin.moc"
