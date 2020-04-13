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

    property QtObject device
    readonly property var _pluginIcons: {
        "kdeconnect_ping": "image://theme/icon-m-accept",
        "kdeconnect_remotecontrol": "image://theme/icon-m-gesture",
        "kdeconnect_remotecommands": "image://theme/icon-m-wizard",
        "kdeconnect_runcommand": "image://theme/icon-m-wizard",
        "sailfishconnect_batteryreport": "image://theme/icon-m-battery",
        "sailfishconnect_clipboard": "image://theme/icon-m-clipboard",
        "sailfishconnect_contacts": "image://theme/icon-m-contact",
        "sailfishconnect_share": "image://theme/icon-m-share",
        "sailfishconnect_telepathy": "image://theme/icon-m-sms",
        "sailfishconnect_sendnotifications": "image://theme/icon-m-notifications",
        "sailfishconnect_mprisremote": "image://theme/icon-m-media",
        "sailfishconnect_remotekeyboard": "image://theme/icon-m-keyboard"
    }

    readonly property var _pluginConfigs: {
        "kdeconnect_runcommand": Qt.resolvedUrl("pluginConfigs/RunCommandsPage.qml")
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
