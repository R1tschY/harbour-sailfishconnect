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

namespace SailfishConnect {

static QString packageType = QStringLiteral("kdeconnect.clipboard");

ClipboardPlugin::ClipboardPlugin(
        Device* device,
        const QString &name,
        const QSet<QString> &outgoingCapabilities)
    : KdeConnectPlugin(device, name, outgoingCapabilities)
{
    QClipboard *clipboard = QGuiApplication::clipboard();

    connect(
        clipboard, &QClipboard::dataChanged,
        this, &ClipboardPlugin::changed);
}

bool ClipboardPlugin::receivePackage(const NetworkPackage &np)
{
    ignoreNextChange_ = true;

    QString content = np.get<QString>(QStringLiteral("content"));
    QGuiApplication::clipboard()->setText(content);
}

void ClipboardPlugin::changed()
{
    if (ignoreNextChange_) {
        ignoreNextChange_ = false;
        return;
    }

    QClipboard *clipboard = QGuiApplication::clipboard();

    NetworkPackage np(packageType, {{"content", clipboard->text()}});
    sendPackage(np);
}

} // namespace SailfishConnect

Q_IMPORT_PLUGIN(ClipboardPluginFactory)
