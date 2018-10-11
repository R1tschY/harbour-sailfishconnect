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

import QtQuick 2.0
import Sailfish.Silica 1.0
import SailfishConnect.Core 0.2

Column {
    visible:
        _device.isReachable && _device.isTrusted &&
        _device.supportedPlugins.indexOf(
            "SailfishConnect::ClipboardPlugin") >= 0

    SectionHeader { text: qsTr("Clipboard") }
    Label {
        wrapMode: Text.Wrap
        width: mainColumn.width
        color: Theme.highlightColor
        font.pixelSize: Theme.fontSizeSmall
        text: qsTr("After copying open the Sailfish Connect window to send " +
                   "the clipboard content. For receiving clipboard content " +
                   "this is not required.")
    }
}
