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
//import SailfishConnect.UI 0.3
import SailfishConnect.Api 0.7
//import SailfishConnect.Mpris 0.3
import "../components"


Page {
    id: page
    objectName: "DevicePage"
    allowedOrientations: Orientation.All

    property string deviceId
    property var _device: daemon.getDevice(deviceId)

    property bool connected: _device && _device.isTrusted && _device.isReachable

    SilicaFlickable {
        id: deviceView
        anchors.fill: parent
        contentHeight: deviceColumn.height + Theme.paddingLarge

        states: [
            State {
                name: "non-reachable"
                when: !_device.isReachable

                PropertyChanges {
                    target: placeholder
                    text: i18n("Device is not reachable")
                }
            },
            State {
                name: "trusted"
                when: _device.isTrusted

                PropertyChanges {
                    target: placeholder
                    text: ""
                }
            },
            State {
                name: "waitingParingRequest"
                when: _device.hasPairingRequests

                PropertyChanges {
                    target: placeholder
                    text: ""
                }
            },
            State {
                name: "waitForAcceptedPairing"
                when: _device.waitsForPairing

                PropertyChanges {
                    target: placeholder
                    text: i18n("Waiting for accepted pairing ...")
                }
            },
            State {
                name: "non-trusted"
                when: !_device.isTrusted

                PropertyChanges {
                    target: placeholder
                    text: ""
                }
            }
        ]

        Column {
            id: deviceColumn
            width: parent.width

            PageHeader {
                id: header
                title: _device ? _device.name : ""
                _titleItem.textFormat: Text.PlainText
            }

            Column {
                id: waitingEntry
                spacing: Theme.paddingLarge
                height: Theme.itemSizeSmall
                width: parent.width - Theme.paddingLarge * 2
                x: Theme.horizontalPageMargin
                visible: deviceView.state === "waitingParingRequest"

                Label {
                    color: Theme.highlightColor
                    text: i18n("This device wants to pair with your device.")
                    width: parent.width
                    wrapMode: Text.Wrap
                }

                Row {
                    id: acceptRejectBtns
                    width: parent.width
                    spacing: Theme.paddingLarge
                    anchors.horizontalCenter: parent.horizontalCenter

                    Button {
                        text: i18n("Accept")
                        onClicked: _device.acceptPairing()
                    }

                    Button {
                        text: i18n("Reject")
                        onClicked: _device.rejectPairing()
                    }
                }
            }

            Column {
                id: trustEntry
                spacing: Theme.paddingLarge
                height: Theme.itemSizeSmall
                width: parent.width - Theme.paddingLarge * 2
                x: Theme.horizontalPageMargin
                visible: deviceView.state === "non-trusted"

                Label {
                    color: Theme.highlightColor
                    text: i18n("Do you want to connect to this device?")
                    width: parent.width
                    wrapMode: Text.Wrap
                }

                Button {
                    id: requestBtn
                    visible: trustEntry.state === ""
                    anchors.horizontalCenter: parent.horizontalCenter

                    text: i18n("Connect")
                    onClicked: {
                        _device.requestPair()
                    }
                }
            }

            Column {
                id: mainColumn
                width: parent.width
                visible: deviceView.state === "trusted"

                // Plugin UIs

                SectionHeader {
                    text: i18n("Actions")
                }
                PluginAction {
                    device: _device
                    title: i18n("Open touchpad")
                    icon: "image://theme/icon-m-gesture"  
                    pluginId: "kdeconnect_remotecontrol"
                    onClicked: openPage("../pages/TouchpadPage.qml")
                }
                PluginAction {
                    device: _device
                    title: i18n("Upload clipboard text")
                    icon: "image://theme/icon-m-clipboard"
                    pluginId: "sailfishconnect_clipboard"
                    onClicked: pluginCall("pushClipboard")
                }
                PluginAction {
                    device: _device
                    title: i18n("Run command...")
                    icon: "image://theme/icon-m-wizard"
                    pluginId: "kdeconnect_remotecommands"
                    onClicked: openPage("../pages/RemoteCommandsPage.qml")
                }
                PluginAction {
                    device: _device
                    title: i18n("Change volume...")
                    icon: "image://theme/icon-m-speaker"
                    pluginId: "kdeconnect_remotesystemvolume"
                    onClicked: openPage("../pages/RemoteVolumePage.qml")
                }
                
                /*ShareUi { id: shareUi }
                RemoteKeyboard { id: remoteKeyboard}

                MprisUi { id: mprisUi }*/
            }

            ViewPlaceholder {
                id: placeholder
                enabled: !!text
                flickable: deviceView
                text: ""
            }
        }

        PullDownMenu {
            MenuItem {
                text: i18n("Encryption info")
                onClicked: pageStack.push(
                               Qt.resolvedUrl("EncryptionInfoPage.qml"),
                               { device: _device })
            }

            MenuItem {
                text: i18n("Plugins")
                onClicked: pageStack.push(
                               Qt.resolvedUrl("DevicePluginsPage.qml"),
                               { device: _device })
            }

            MenuItem {
                visible: _device && _device.isTrusted
                text: i18n("Unpair")
                onClicked: _device.unpair()
            }

            MenuItem {
                visible: connected
                text: i18n("Send ping")
                onClicked: _device.pluginCall("ping", "sendPing")
            }
        }


        VerticalScrollDecorator { flickable: deviceView }
    }
}
