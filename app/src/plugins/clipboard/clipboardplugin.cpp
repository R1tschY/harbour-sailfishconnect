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

#include "clipboardplugin.h"

#include <QGuiApplication>
#include <QClipboard>
#include <sailfishconnect/networkpacket.h>

namespace SailfishConnect {

static QString packetType = QStringLiteral("kdeconnect.clipboard");

ClipboardPlugin::ClipboardPlugin(
        Device* device,
        const QString &name,
        const QSet<QString> &outgoingCapabilities)
    : KdeConnectPlugin(device, name, outgoingCapabilities)
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

QString ClipboardPluginFactory::name() const
{
    return tr("Clipboard");
}

QString ClipboardPluginFactory::description() const
{
    return tr("Synchronize clipboard between devices.");
}

QString ClipboardPluginFactory::iconUrl() const
{
    return "image://theme/icon-m-clipboard";
}

} // namespace SailfishConnect

Q_IMPORT_PLUGIN(ClipboardPluginFactory)
