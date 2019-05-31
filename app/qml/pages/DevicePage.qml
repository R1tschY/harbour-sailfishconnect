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
import SailfishConnect.Core 0.3
import SailfishConnect.Mpris 0.3
import "../components"


Page {
    id: page
    objectName: "DevicePage"
    allowedOrientations: Orientation.All

    property string deviceId
    property Device _device: daemon.getDevice(deviceId)

    property bool connected: _device.isTrusted && _device.isReachable

    SilicaFlickable {
        id: deviceView
        anchors.fill: parent
        contentHeight: deviceColumn.height + Theme.paddingLarge

        Column {
            id: deviceColumn
            width: parent.width

            states: [
                State {
                    name: "non-reachable"
                    when: !_device.isReachable
                },
                State {
                    name: "trusted"
                    when: _device.isTrusted
                },
                State {
                    name: "waitingParingRequest"
                    when: _device.hasPairingRequests
                },
                State {
                    name: "waitForAcceptedPairing"
                    when: _device.waitsForPairing
                },
                State {
                    name: "non-trusted"
                    when: !_device.isTrusted
                }
            ]

            PageHeader {
                id: header
                title: _device.name
                _titleItem.textFormat: Text.PlainText
            }

            Column {
                id: waitingEntry
                spacing: Theme.paddingLarge
                height: Theme.itemSizeSmall
                width: parent.width - Theme.paddingLarge * 2
                x: Theme.horizontalPageMargin
                visible: deviceColumn.state === "waitingParingRequest"

                Label {
                    color: Theme.highlightColor
                    text: qsTr("This device wants to pair with your device.")
                    // qsTr("Waiting for accepted pairing ...")
                }

                Row {
                    id: acceptRejectBtns
                    width: parent.width
                    spacing: Theme.paddingLarge
                    anchors.horizontalCenter: parent.horizontalCenter

                    Button {
                        text: qsTr("Accept")
                        onClicked: _device.acceptPairing()
                    }

                    Button {
                        text: qsTr("Reject")
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
                visible: deviceColumn.state === "non-trusted"

                Label {
                    color: Theme.highlightColor
                    text: qsTr("Do you want to connect to this device?")
                }

                Button {
                    id: requestBtn
                    visible: trustEntry.state === ""
                    anchors.horizontalCenter: parent.horizontalCenter

                    text: qsTr("Connect")
                    onClicked: {
                        _device.requestPair()
                    }
                }
            }

            Column {
                id: mainColumn
                width: parent.width
                visible: deviceColumn.state === "trusted"

                // Plugin UIs

                ShareUi { id: shareUi }
                MprisUi { id: mprisUi }
                Touchpad { id: touchpad }
                ClipboardUi { id: clipboardUi }
                RemoteKeyboard { id: remoteKeyboard}
            }
        }

        Label {
            visible: deviceColumn.state === "non-reachable"
            anchors.fill: parent

            text: qsTr("Device is not reachable")
        }

        Label {
            visible: _device.isReachable && !_device.isTrusted
            anchors.fill: parent

            text: "TODO: device settings?"
        }

        PullDownMenu {
            MenuItem {
                text: qsTr("Encryption info")
                onClicked: pageStack.push(
                               Qt.resolvedUrl("EncryptionInfoPage.qml"),
                               { device: _device })
            }

            MenuItem {
                text: qsTr("Plugins")
                onClicked: pageStack.push(
                               Qt.resolvedUrl("DevicePluginsPage.qml"),
                               { device: deviceId })
            }

            MenuItem {
                visible: _device.isTrusted
                text: qsTr("Unpair")
                onClicked: _device.unpair()
            }

            MenuItem {
                visible: connected
                text: qsTr("Send ping")
                onClicked: _device.plugin("SailfishConnect::PingPlugin").
                    sendPing()
            }
        }


        VerticalScrollDecorator { flickable: deviceView }
    }
}
