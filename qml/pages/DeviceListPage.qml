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
import Nemo.Notifications 1.0
import SailfishConnect.UI 0.1

Page {
    id: page

    function selectDeviceColor(trusted, reachable) {
        return (reachable
            ? Theme.highlightColor
            : Theme.primaryColor)
    }

    allowedOrientations: Orientation.All

    SilicaFlickable {
        anchors.fill: parent

        contentHeight: column.height

        Column {
            id: column

            width: page.width
            spacing: Theme.paddingLarge
            PageHeader {
                title: qsTr("Select device")
            }

            Component {
                id: deviceDelegate

                ListItem {
                    id: listItem
                    width: page.width
                    height: Theme.itemSizeMedium

                    Image {
                        id: icon
                        source: iconUrl + "?" +
                                selectDeviceColor(trusted, reachable)

                        x: Theme.horizontalPageMargin
                        anchors.verticalCenter: parent.verticalCenter
                        sourceSize.width: Theme.iconSizeMedium
                        sourceSize.height: Theme.iconSizeMedium
                    }
                    Column {
                        anchors {
                            left: icon.right
                            leftMargin: Theme.paddingLarge
                            right: parent.right
                            rightMargin: Theme.horizontalPageMargin
                            verticalCenter: parent.verticalCenter
                        }

                        Label {
                            text: '<b>' + name + '</b>'
                            color: listItem.highlighted
                                   ? Theme.highlightColor
                                   : Theme.primaryColor
                            truncationMode: TruncationMode.Fade
                        }
                        Label {
                            text: id
                            color: listItem.highlighted
                                   ? Theme.secondaryHighlightColor
                                   : Theme.secondaryColor
                            font.pixelSize: Theme.fontSizeExtraSmall
                            width: parent.width
                            truncationMode: TruncationMode.Fade
                        }
                    }

                    onClicked: {
                        pageStack.push(
                            Qt.resolvedUrl("DevicePage.qml"),
                            { deviceId: id })
                    }
                }
            }


            DeviceListModel {
                id: devicelistModel
            }

            // Test model
//            ListModel {
//                id: devicelistModel

//                ListElement { iconUrl: "image://theme/icon-m-phone"; name: "1"; deviceId: "1"; trusted: true; reachable: true }
//                ListElement { iconUrl: "image://theme/icon-m-phone"; name: "2"; deviceId: "1"; trusted: true; reachable: true }
//                ListElement { iconUrl: "image://theme/icon-m-phone"; name: "3"; deviceId: "1"; trusted: true; reachable: true }
//                ListElement { iconUrl: "image://theme/icon-m-phone"; name: "4"; deviceId: "1"; trusted: true; reachable: false }
//                ListElement { iconUrl: "image://theme/icon-m-phone"; name: "5"; deviceId: "1"; trusted: true; reachable: false }

//                ListElement { iconUrl: "image://theme/icon-m-phone"; name: "1"; deviceId: "1"; trusted: false; reachable: true }
//                ListElement { iconUrl: "image://theme/icon-m-phone"; name: "2"; deviceId: "1"; trusted: false; reachable: true }
//                ListElement { iconUrl: "image://theme/icon-m-phone"; name: "3"; deviceId: "1"; trusted: false; reachable: true }
//                ListElement { iconUrl: "image://theme/icon-m-phone"; name: "4"; deviceId: "1"; trusted: false; reachable: true }
//                ListElement { iconUrl: "image://theme/icon-m-phone"; name: "5"; deviceId: "1"; trusted: false; reachable: true }
//             }

            FilterValueProxyModel {
                id: trustedDevicesModel

                filterRoleName: "trusted"
                filterValue: true
                sourceModel: devicelistModel
            }

            FilterValueProxyModel {
                id: nontrustedDevicesModel

                filterRoleName: "trusted"
                filterValue: false
                sourceModel: devicelistModel
            }

            SectionHeader { text: qsTr("Trusted devices") }
            SilicaListView {
                width: page.width
                height: childrenRect.height

                model: trustedDevicesModel
                delegate: deviceDelegate
            }

            SectionHeader { text: qsTr("Near devices") }
            SilicaListView {
                width: page.width
                height: childrenRect.height

                model: nontrustedDevicesModel
                delegate: deviceDelegate
            }
        }

        VerticalScrollDecorator {}
    }
}

