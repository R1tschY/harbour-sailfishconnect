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
import SailfishConnect.UI 0.3
import SailfishConnect.Api 0.7


Page {
    id: page

    property var device
    readonly property var _pluginIcons: {
        "kdeconnect_ping": "image://theme/icon-m-accept",
        "sailfishconnect_batteryreport": "image://theme/icon-m-battery",
        "sailfishconnect_ping": "image://theme/icon-m-clipboard",
        "sailfishconnect_contacts": "image://theme/icon-m-contact",
        "sailfishconnect_share": "image://theme/icon-m-share",
        "sailfishconnect_telepathy": "image://theme/icon-m-sms",
        "sailfishconnect_telephony": "image://theme/icon-m-answer",
    }

    allowedOrientations: Orientation.All

    SilicaListView {
        id: listView
        model: SortFilterModel {
            sortRole: "pluginName"
            filterMode: "none"
            sourceModel: DevicePluginsModel { }
            Component.onCompleted: sourceModel.device = device
        }

        anchors.fill: parent
        header: PageHeader {
            title: qsTr("Select Plugins")
        }

        delegate: BackgroundItem {
            id: delegate
            width: page.width
            height: pluginSwitch.height

            IconTextSwitch {
                id: pluginSwitch
                text: pluginName
                icon.source: _pluginIcons[pluginId] || ""
                description: pluginDescription
                onCheckedChanged: pluginEnabled = checked

                Component.onCompleted: pluginSwitch.checked = pluginEnabled
            }
        }

        VerticalScrollDecorator {}
    }
}
