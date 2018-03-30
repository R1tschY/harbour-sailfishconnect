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
import SailfishConnect.UI 0.1


Page {
    id: page

    property string device

    allowedOrientations: Orientation.All

    SilicaListView {
        id: listView
        model: DevicePluginsModel {
            deviceId: device
        }
        anchors.fill: parent
        header: PageHeader {
            title: qsTr("Select Plugins")
        }

        delegate: ListItem {
            id: delegate
            width: page.width
            height: pluginSwitch.height

            IconTextSwitch {
                id: pluginSwitch
                text: pluginName
                icon.source: pluginIconUrl
                description: pluginDescription
                onCheckedChanged: pluginEnabled = checked

                Component.onCompleted: pluginSwitch.checked = pluginEnabled
            }
        }

        VerticalScrollDecorator {}
    }
}
