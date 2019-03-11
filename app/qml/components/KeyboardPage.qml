/*
 * Copyright 2019 Lieven Hey <t.schilling@snafu.de>.
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

Page {
    id: page
    allowedOrientations: Orientation.Landscape | Orientation.LandscapeInverted
    property var device
    property var plugin: device.plugin("SailfishConnect::RemoteInputPlugin")
    property QtObject modifiers: QtObject {
        property int shift: 0
        property int ctrl: 0
        property int alt: 0
    }

    PageHeader {
        title: qsTr("Keyboard")
    }

    SilicaFlickable {
        anchors.fill: parent

        Column {
            anchors.bottom: parent.bottom

            Row {
                Repeater {
                    model: _keyboardLayout.row1

                    Key {
                        width: page.width / _keyboardLayout.row1.length * modelData["width"]
                        height: page.height / 7
                        label: modelData["name"] === "backspace" ? "" : modelData["name"]
                        altLabel: modelData["alt"]
                        showAlt: modifiers.shift

                        Image {
                            anchors.centerIn: parent
                            width: parent.width
                            height: parent.height
                            fillMode: Image.PreserveAspectFit
                            visible: modelData["name"] === "backspace"

                            source: "image://theme/icon-m-backspace"
                        }

                        onClicked: {
                            if (modelData["name"] === "backspace") {
                                plugin.sendKeyPress("backspace", modifiers.shift, modifiers.ctrl, modifiers.atl);
                            } else {
                                plugin.sendKeyPress(modifiers.shift ? altLabel : label, false, modifiers.ctrl, modifiers.alt);
                            }
                            if (modifiers.shift == 1) {
                                modifiers.shift = 0;
                            }
                            if (modifiers.ctrl == 1) {
                                modifiers.ctrl = 0;
                            }
                            if (modifiers.alt == 1) {
                                modifiers.alt = 0;
                            }
                        }
                    }
                }
            }

            Row {
                Repeater {
                    model: _keyboardLayout.row2

                    Key {
                        width: page.width / _keyboardLayout.row1.length * modelData["width"]
                        height: page.height / 7
                        label: modelData["name"]
                        altLabel: modelData["alt"]
                        showAlt: modifiers.shift

                        onClicked: {
                            plugin.sendKeyPress(modifiers.shift ? altLabel : label, false, modifiers.ctrl, modifiers.alt);
                            if (modifiers.shift == 1) {
                                modifiers.shift = 0;
                            }
                            if (modifiers.ctrl == 1) {
                                modifiers.ctrl = 0;
                            }
                            if (modifiers.alt == 1) {
                                modifiers.alt = 0;
                            }
                        }
                    }
                }
            }

            Row {
                Repeater {
                    model: _keyboardLayout.row3

                    Key {
                        width: page.width / _keyboardLayout.row1.length * modelData["width"]
                        height: page.height / 7
                        label: modelData["name"] === "enter" ? "" : modelData["name"]
                        altLabel: modelData["alt"]
                        showAlt: modifiers.shift

                        Image {
                            anchors.centerIn: parent
                            width: parent.width
                            height: parent.height
                            fillMode: Image.PreserveAspectFit
                            visible: modelData["name"] === "enter"

                            source: "image://theme/icon-m-enter"
                        }

                        onClicked: {
                            if (modelData["name"] === "enter") {
                                plugin.sendKeyPress("enter", modifiers.shift, modifiers.ctrl, modifiers.alt);
                            } else {
                                plugin.sendKeyPress(label, modifiers.shift, modifiers.ctrl, modifiers.alt)
                            }
                            if (modifiers.shift == 1) {
                                modifiers.shift = 0;
                            }
                            if (modifiers.ctrl == 1) {
                                modifiers.ctrl = 0;
                            }
                            if (modifiers.alt == 1) {
                                modifiers.alt = 0;
                            }
                        }
                    }
                }
            }

            Row {
                Repeater {
                    model: _keyboardLayout.row4

                    Key {
                        width: page.width / _keyboardLayout.row1.length * modelData["width"]
                        height: page.height / 7
                        label: modelData["name"]
                        altLabel: modelData["alt"]
                        showAlt: modifiers.shift

                        onClicked: {
                            plugin.sendKeyPress(label, modifiers.shift, modifiers.ctrl, modifiers.alt)
                            if (modifiers.shift == 1) {
                                modifiers.shift = 0;
                            }
                            if (modifiers.ctrl == 1) {
                                modifiers.ctrl = 0;
                            }
                            if (modifiers.alt == 1) {
                                modifiers.alt = 0;
                            }
                        }
                    }
                }
            }

            Row {
                Repeater {
                    model: _keyboardLayout.row5

                    Key {
                        id: key
                        width: page.width / _keyboardLayout.row1.length * modelData["width"]
                        height: page.height / 7
                        label: modelData["name"] === "up" ? "" : modelData["name"]
                        altLabel: modelData["alt"]
                        showAlt: modifiers.shift
                        markable: label == "shift"
                        markValue: modifiers.shift

                        Image {
                            anchors.centerIn: parent
                            width: parent.width
                            height: parent.height
                            fillMode: Image.PreserveAspectFit
                            visible: modelData["name"] === "up"

                            source: "image://theme/icon-m-up"
                        }

                        onClicked: {
                            if (label === "shift") {
                                modifiers.shift++;
                                if (modifiers.shift == 3) {
                                    modifiers.shift = 0;
                                }
                                return;
                            } else if (modelData["name"] === "up") {
                                plugin.sendKeyPress("up", modifiers.shift, modifiers.ctrl, modifiers.alt);
                            } else {
                                plugin.sendKeyPress(label, modifiers.shift, modifiers.ctrl, modifiers.alt)
                            }
                            if (modifiers.shift == 1) {
                                modifiers.shift = 0;
                            }
                            if (modifiers.ctrl == 1) {
                                modifiers.ctrl = 0;
                            }
                            if (modifiers.alt == 1) {
                                modifiers.alt = 0;
                            }
                        }
                    }
                }
            }

            Row {
                Repeater {
                    model: _keyboardLayout.row6

                    Key {
                        width: page.width / _keyboardLayout.row1.length * modelData["width"]
                        height: page.height / 7
                        label: modelData["name"] === "left" || modelData["name"] === "down" || modelData["name"] === "right" ? "" : modelData["name"]
                        altLabel: modelData["alt"]
                        showAlt: modifiers.shift
                        markable: label == "ctrl" || label == "alt"
                        markValue: label == "ctrl" ? modifiers.ctrl : modifiers.alt

                        Image {
                            anchors.centerIn: parent
                            width: parent.width
                            height: parent.height
                            fillMode: Image.PreserveAspectFit
                            visible: modelData["name"] === "left" || modelData["name"] === "down" || modelData["name"] === "right"

                            source: modelData["name"] === "left" ? "image://theme/icon-m-left" :  modelData["name"] === "down" ? "image://theme/icon-m-down" : "image://theme/icon-m-right"
                        }

                        onClicked: {
                            if (modelData["name"] === "left") {
                                plugin.sendKeyPress("left", modifiers.shift, modifiers.ctrl, modifiers.alt);
                            } else if (modelData["name"] === "down") {
                                plugin.sendKeyPress("down", modifiers.shift, modifiers.ctrl, modifiers.alt);
                            } else if (modelData["name"] === "right") {
                                plugin.sendKeyPress("right", modifiers.shift, modifiers.ctrl, modifiers.alt);
                            } else if (label == "ctrl") {
                                modifiers.ctrl++;
                                if (modifiers.ctrl == 3) {
                                    modifiers.ctrl = 0;
                                }
                                return;
                            } else if (label == "alt") {
                                modifiers.alt++;
                                if (modifiers.alt == 3) {
                                    modifiers.alt = 0;
                                }
                                return;
                            } else {
                                plugin.sendKeyPress(label, modifiers.shift, modifiers.ctrl, modifiers.alt)
                            }
                            if (modifiers.shift == 1) {
                                modifiers.shift = 0;
                            }
                            if (modifiers.ctrl == 1) {
                                modifiers.ctrl = 0;
                            }
                            if (modifiers.alt == 1) {
                                modifiers.alt = 0;
                            }
                        }
                    }
                }
            }
        }
    }

    // I hate this hack but it seems this is the only way to push an attachted page
    Timer {
        id: pushPage

        interval: 50
        repeat: false
        running: true

        onTriggered: {
            pageStack.pushAttached(Qt.resolvedUrl("KeyboardChangeLayoutPage.qml"))
        }
    }
}
