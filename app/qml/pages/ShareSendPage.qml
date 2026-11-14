/*
 * Copyright 2026 Vivia Nikolaidou <vivia@ahiru.eu>
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

    property string filePath
    readonly property string fileName:
        filePath.length > 0 ? filePath.split("/").pop() : ""

    allowedOrientations: Orientation.All

    DBusServiceWatcher {
        id: kdeconnectService
        service: "org.kde.kdeconnect"
    }

    BusyIndicator {
        size: BusyIndicatorSize.Large
        anchors.centerIn: parent
        running: !kdeconnectService.registered
    }

    SilicaFlickable {
        id: content
        anchors.fill: parent
        contentHeight: column.height
        visible: kdeconnectService.registered

        ViewPlaceholder {
            enabled: connectedDevices.count === 0
            text: i18n("No connected device that receives files was found. "
                       + "Pair a device and enable file sharing on it.")
        }

        Column {
            id: column

            width: page.width
            spacing: Theme.paddingLarge

            PageHeader {
                title: i18n("Send file")
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                text: page.fileName
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                truncationMode: TruncationMode.Fade
                textFormat: Text.PlainText
            }

            SectionHeader {
                text: i18n("Connected devices")
            }

            DeviceListModel {
                id: devicelistModel
            }

            SortFilterModel {
                id: pairedDevicesModel

                sortRole: "name"
                filterRole: "paired"
                filterValue: true
                sourceModel: devicelistModel
            }

            SortFilterModel {
                id: connectedDevicesModel

                filterRole: "reachable"
                filterValue: true
                sourceModel: pairedDevicesModel
            }

            SortFilterModel {
                id: shareCapableDevicesModel

                filterRole: "shareSupported"
                filterValue: true
                sourceModel: connectedDevicesModel
            }

            ColumnView {
                id: connectedDevices
                width: page.width
                itemHeight: Theme.itemSizeMedium

                model: shareCapableDevicesModel
                delegate: deviceDelegate
            }
        }

        Component {
            id: deviceDelegate

            ListItem {
                width: page.width
                contentHeight: Theme.itemSizeMedium

                Image {
                    id: icon
                    source: iconUrl

                    x: Theme.horizontalPageMargin
                    anchors.verticalCenter: parent.verticalCenter
                    sourceSize.width: Theme.iconSizeMedium
                    sourceSize.height: Theme.iconSizeMedium
                }

                Label {
                    anchors {
                        left: icon.right
                        leftMargin: Theme.paddingLarge
                        right: parent.right
                        rightMargin: Theme.horizontalPageMargin
                        verticalCenter: parent.verticalCenter
                    }

                    text: name
                    color: highlighted
                           ? Theme.highlightColor
                           : Theme.primaryColor
                    truncationMode: TruncationMode.Fade
                    textFormat: Text.PlainText
                }

                onClicked: page.sendToDevice(id)
            }
        }

        PullDownMenu {
            MenuItem {
                text: i18n("Refresh")
                onClicked: daemon.forceOnNetworkChange()
            }
        }

        VerticalScrollDecorator { flickable: content }
    }

    function sendToDevice(deviceId) {
        console.log("Sending " + filePath + " to device " + deviceId)

        var device = daemon.getDevice(deviceId)
        if (!device) {
            console.warn("Sending to non-existing device "
                         + deviceId + " ignored")
            return;
        }

        // file:// prefix is required: the Share plugin wraps the string in
        // a QUrl and distinguishes local files by checking isLocalFile().
        device.getShareApi().shareUrl("file://" + filePath)
        pageStack.pop()
    }
}
