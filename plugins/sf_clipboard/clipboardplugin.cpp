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
#include <QDateTime>
#include <core/networkpacket.h>

K_PLUGIN_CLASS_WITH_JSON(ClipboardPlugin, "sailfishconnect_clipboard.json")

static QString PACKET_TYPE_CLIPBOARD = QStringLiteral("kdeconnect.clipboard");
static QString PACKET_TYPE_CLIPBOARD_CONNECT = QStringLiteral("kdeconnect.clipboard.connect");

ClipboardPlugin::ClipboardPlugin(QObject* parent, const QVariantList& args)
    : KdeConnectPlugin(parent, args)
    , m_clipboard(QGuiApplication::clipboard())
{}

void ClipboardPlugin::receivePacket(const NetworkPacket &np)
{
    QString content = np.get<QString>(QStringLiteral("content"));
    if (np.type() == PACKET_TYPE_CLIPBOARD) {
        m_currentContent = content;
        m_updateTimestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
        m_clipboard->setText(content);
    } else if (np.type() == PACKET_TYPE_CLIPBOARD_CONNECT) {
        qint64 timestamp = np.get<qint64>(QStringLiteral("timestamp"));
        if (timestamp > m_updateTimestamp) {
            m_currentContent = content;
            m_updateTimestamp = timestamp;
            m_clipboard->setText(content);
        }
    }
}

void ClipboardPlugin::pushClipboard()
{
    m_updateTimestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
    m_currentContent = m_clipboard->text();

    NetworkPacket np(PACKET_TYPE_CLIPBOARD, {
        {QStringLiteral("content"), m_currentContent},
    });
    sendPacket(np);
}

QString ClipboardPlugin::dbusPath() const
{
    return QStringLiteral("/modules/kdeconnect/devices/%1/sailfishconnect_clipboard")
        .arg(device()->id());
}

#include "clipboardplugin.moc"
