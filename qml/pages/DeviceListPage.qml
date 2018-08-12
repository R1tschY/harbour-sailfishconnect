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
import SailfishConnect.UI 0.2

Page {
    id: page

    allowedOrientations: Orientation.All

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        Column {
            id: column

            width: page.width
            spacing: Theme.paddingLarge

            PageHeader {
                title: qsTr("Sailfish-Connect")
            }

            TextField {
                width: parent.width
                label: qsTr("Device Name")
                placeholderText: daemon.announcedName()
                text: daemon.announcedName()

                onActiveFocusChanged: {
                    if (activeFocus)
                        return

                    if (text.length === 0) {
                        text = daemon.announcedName()
                    } else {
                        daemon.setAnnouncedName(text)
                        placeholderText = text
                    }
                }

                EnterKey.onClicked: page.focus = true
                EnterKey.iconSource: "image://theme/icon-m-enter-close"
            }


            Component {
                id: deviceDelegate

                ListItem {
                    id: listItem

                    property bool showStatus: deviceStatusLabel.text.length

                    width: page.width
                    height: Theme.itemSizeMedium

                    Image {
                        id: icon
                        source: iconUrl

                        x: Theme.horizontalPageMargin
                        anchors.verticalCenter: parent.verticalCenter
                        sourceSize.width: Theme.iconSizeMedium
                        sourceSize.height: Theme.iconSizeMedium
                    }

                    Label {
                        id: deviceNameLabel
                        anchors {
                            left: icon.right
                            leftMargin: Theme.paddingLarge
                            right: parent.right
                            rightMargin: Theme.horizontalPageMargin
                        }
                        y: listItem.contentHeight / 2 - implicitHeight / 2
                           - showStatus * (deviceStatusLabel.implicitHeight / 2)

                        text: name
                        color: listItem.highlighted
                               ? Theme.highlightColor
                               : Theme.primaryColor
                        truncationMode: TruncationMode.Fade
                        textFormat: Text.PlainText

                        Behavior on y { NumberAnimation {} }
                    }

                    Label {
                        id: deviceStatusLabel
                        anchors {
                            left: deviceNameLabel.left
                            top: deviceNameLabel.bottom
                            right: parent.right
                            rightMargin: Theme.horizontalPageMargin
                        }

                        text: (trusted && reachable)
                              ? qsTr("Connected")
                              : (hasPairingRequests
                                 ? qsTr("Pending pairing request ...") : "")
                        color: listItem.highlighted
                               ? Theme.secondaryHighlightColor
                               : Theme.secondaryColor
                        truncationMode: TruncationMode.Fade
                        font.pixelSize: Theme.fontSizeExtraSmall
                        opacity: showStatus ? 1.0 : 0.0
                        width: parent.width
                        textFormat: Text.PlainText

                        Behavior on opacity { FadeAnimation {} }
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
            FilterValueProxyModel {
                id: otherNearDevicesModel

                filterRoleName: "reachable"
                filterValue: true
                sourceModel: nontrustedDevicesModel
            }

            SilicaListView {
                width: page.width
                height: contentItem.childrenRect.height
                header: SectionHeader { text: qsTr("Paired devices") }

                model: trustedDevicesModel
                delegate: deviceDelegate
            }


            SilicaListView {
                width: page.width
                height: contentItem.childrenRect.height
                header: SectionHeader { text: qsTr("Nearby devices") }

                model: otherNearDevicesModel
                delegate: deviceDelegate
            }
        }

        PullDownMenu {
            MenuItem {
                text: qsTr("About ...")
                onClicked: pageStack.push(Qt.resolvedUrl("AboutPage.qml"))
            }

            MenuItem {
                text: qsTr("Refresh")
                onClicked: daemon.forceOnNetworkChange()
            }
        }

        VerticalScrollDecorator {}
    }
}

