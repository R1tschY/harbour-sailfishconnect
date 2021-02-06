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
import SailfishConnect.UI 0.6
import SailfishConnect.Api 0.6

Page {
    id: page

    property bool startup: true

    allowedOrientations: Orientation.All

    DBusServiceWatcher {
        id: kdeconnectService
        service: "org.kde.kdeconnect"

        onRegisteredChanged: {
            if (kdeconnectService.registered) {
                announcedNameField.text = daemon.announcedName()
            }
        }
    }

    BusyIndicator {
        size: BusyIndicatorSize.Large
        anchors.centerIn: parent
        running: !kdeconnectService.registered
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height
        visible: kdeconnectService.registered

        ViewPlaceholder {
            enabled: !startup
                     && trustedDevices.count === 0
                     && nearDevices.count === 0
            text: i18n("Install KDE Connect or GSConnect on your computer and connect it to the same WLAN.")
        }

        Column {
            id: column

            width: page.width
            spacing: Theme.paddingLarge

            PageHeader {
                title: i18n("Sailfish Connect")  // TODO: QT5.9: Qt.application.displayName
            }

            TextField {
                id: announcedNameField
                width: parent.width
                label: i18n("Device Name")
                text: kdeconnectService.registered ? daemon.announcedName() : ""

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

                EnterKey.onClicked: announcedNameField.focus = false
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
                              ? i18n("Connected")
                              : (hasPairingRequests || waitsForPairing
                                 ? i18n("Pending pairing request ...") : "")
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

            SortFilterModel {
                id: trustedDevicesModel

                sortRole: "name"
                filterRole: "trusted"
                filterValue: true
                sourceModel: devicelistModel
            }

            SortFilterModel {
                id: nontrustedDevicesModel

                sortRole: "name"
                filterRole: "trusted"
                filterValue: false
                sourceModel: devicelistModel
            }
            SortFilterModel {
                id: otherNearDevicesModel

                filterRole: "reachable"
                filterValue: true
                sourceModel: nontrustedDevicesModel
            }

            SectionHeader {
                text: i18n("Paired devices")
                visible: trustedDevices.count > 0
            }
            ColumnView {
                id: trustedDevices
                width: page.width
                itemHeight: Theme.itemSizeMedium


                model: trustedDevicesModel
                delegate: deviceDelegate
                visible: trustedDevices.count > 0
            }

            SectionHeader {
                text: i18n("Nearby devices")
                visible: nearDevices.count > 0
            }
            ColumnView {
                id: nearDevices
                width: page.width
                itemHeight: Theme.itemSizeMedium

                model: otherNearDevicesModel
                delegate: deviceDelegate
                visible: nearDevices.count > 0
            }
        }

        PullDownMenu {
            MenuItem {
                text: i18n("About ...")
                onClicked: pageStack.push(Qt.resolvedUrl("AboutPage.qml"))
            }

            MenuItem {
                text: i18n("Settings ...")
                onClicked: pageStack.push(Qt.resolvedUrl("SettingsPage.qml"))
            }

            MenuItem {
                text: i18n("Quit")
                visible: ui.runInBackground
                onClicked: ui.quit()
            }

            MenuItem {
                text: i18n("Refresh")
                onClicked: daemon.forceOnNetworkChange()
            }
        }

        VerticalScrollDecorator {}
    }

    Connections {
        target: ui
        onOpeningDevicePage: openDevicePage(deviceId)
    }

    Timer {
        interval: 1000
        running: true
        repeat: false
        onTriggered: startup = false
    }

    function openDevicePage(deviceId) {
        if (typeof pageStack === "undefined")
            return; // happens when using QmlLive

        console.log("opening device " + deviceId)

        window.activate()

        var devicePage = pageStack.find(function(page) {
            return page.objectName === "DevicePage"
        })
        if (devicePage !== null && devicePage.deviceId === deviceId) {
            pageStack.pop(devicePage)
            ui.showMainWindow()
            return
        }

        pageStack.pop(page, PageStackAction.Immediate)
        pageStack.push(
            Qt.resolvedUrl("DevicePage.qml"),
            { deviceId: deviceId },
            PageStackAction.Immediate)
    }
}

