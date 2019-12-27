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

import QtQuick 2.0
import Sailfish.Silica 1.0
import SailfishConnect.Api 0.7

IconListItem {
    title: qsTr("Keyboard")
    source: "image://theme/icon-m-keyboard"
    visible: _device.loadedPlugins.indexOf(
                 "SailfishConnect::RemoteKeyboardPlugin") >= 0

    onClicked: pageStack.push(
                   Qt.resolvedUrl("../pages/KeyboardPage.qml"),
                   { device: _device });
}
