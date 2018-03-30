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
import SailfishConnect.Core 0.1


Page {
    id: page

    property string deviceId
    property Device _device: daemon.getDevice(deviceId)
    property bool waitForAcceptedPairing: false

    SilicaListView {
        anchors.fill: parent

        header: Column {
            width: parent.width
            height: header.height + mainColumn.height + Theme.paddingLarge

            PageHeader {
                id: header
                title: _device.name
            }

            function updateState() {
                waitForAcceptedPairing = false
            }

            Component.onCompleted: {
                _device.pairingError.connect(updateState)
                _device.trustedChanged.connect(updateState)
            }

            Column {
                id: mainColumn
                width: parent.width - Theme.paddingLarge * 2
                spacing: Theme.paddingLarge
                x: Theme.paddingLarge

                Label {
                    id: stateText
                    text: qsTr("You don't trust this device")
                }

                Button {
                    id: requestBtn
                    visible: mainColumn.state == ""
                    anchors.horizontalCenter: parent.horizontalCenter

                    text: qsTr("Request pairing")
                    onClicked: {
                        waitForAcceptedPairing = true
                        _device.requestPair()
                    }
                }

                Row {
                    id: acceptRejectBtns
                    visible: mainColumn.state == "waitingParingRequest"
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

                states: [
                    State {
                        name: "trusted"
                        when: _device.isTrusted
                        PropertyChanges {
                            target: stateText;
                            text: qsTr("You trust this device")}
                    },
                    State {
                        name: "waitForAcceptedPairing"
                        when: waitForAcceptedPairing
                        PropertyChanges {
                            target: stateText;
                            text: qsTr("Waiting for accepted pairing ...")}
                    },
                    State {
                        name: "waitingParingRequest"
                        when: _device.hasPairingRequests
                        PropertyChanges {
                            target: stateText;
                            text: qsTr("This device wants to pair with your " +
                                       "device.")}
                    }
                ]
            }
        }

        model: 0 // TODO: plugin UIs

        PullDownMenu {
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
        }

        VerticalScrollDecorator {}
    }
}
