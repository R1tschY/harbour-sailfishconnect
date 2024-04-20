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
import SailfishConnect.UI 0.6
import SailfishConnect.Api 0.6

Page {
    id: page

    property QtObject device
    readonly property var _pluginIcons: {
        "Ping": "image://theme/icon-m-accept",
        "RemoteControl": "image://theme/icon-m-gesture",
        "RemoteKeyboard": "image://theme/icon-m-keyboard",
        "RemoteCommands": "image://theme/icon-m-wizard",
        "RunCommand": "image://theme/icon-m-wizard",
        "RemoteSystemVolume": "image://theme/icon-m-speaker",
        "BatteryReport": "image://theme/icon-m-battery",
        "Clipboard": "image://theme/icon-m-clipboard",
        "Contacts": "image://theme/icon-m-contact",
        "Share": "image://theme/icon-m-share",
        "Telepathy": "image://theme/icon-m-sms",
        "SendNotifications": "image://theme/icon-m-notifications",
        "MprisRemote": "image://theme/icon-m-media"
    }

    readonly property var _pluginConfigs: {
        "RunCommand": Qt.resolvedUrl("pluginConfigs/RunCommandsConfigPage.qml"),
        "Share": Qt.resolvedUrl("pluginConfigs/ShareConfigPage.qml"),
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
            title: i18n("Select Plugins")
        }

        delegate: ListItem {
            id: delegate

            property bool hasConfig: !!_pluginConfigs[pluginId]

            width: page.width
            contentHeight: pluginSwitch.height
            menu: hasConfig ? contextMenu : null

            IconTextSwitch {
                id: pluginSwitch
                text: pluginName
                icon.source: _pluginIcons[pluginId] || ""
                description: pluginDescription 
                    + (hasConfig ? "<br><i>%1</i>".arg(i18n("Open configuration throught context menu")) : "")
                onCheckedChanged: pluginEnabled = checked
                onPressAndHold: openMenu()

                Component.onCompleted: pluginSwitch.checked = pluginEnabled
            }

            Component {
                id: contextMenu
                ContextMenu {
                    MenuItem {
                        text: i18n("Open configuration")
                        onClicked: pageStack.push(
                            _pluginConfigs[pluginId],
                            { device: device })
                    }
                }
            }
        }

        VerticalScrollDecorator {}
    }
}
