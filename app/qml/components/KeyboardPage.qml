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
    property bool chars: false

    function displayCorrectChar(data) {
        if (page.chars && typeof data["symView"] !== "undefined") {
            // dont show text on arrow keys
            if (data["symView"] === "up" || data["symView"] === "left" || data["symView"] === "down" || data["symView"] === "right") return " ";
            if (modifiers.shift && typeof data["symView2"] !== "undefined") {
                return data["symView2"];
            }
            return data["symView"];
        } else {
            if (modifiers.shift && typeof data["captionShifted"] !== "undefined") {
                return data["captionShifted"];
            }
            return data["caption"];
        }
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
                        width: page.width / _keyboardLayout.row1.length
                        height: page.height / 7
                        label: displayCorrectChar(modelData)

                        onClicked: {
                            if (page.chars) {
                                plugin.sendKeyPress(label, false, modifiers.ctrl, modifiers.alt)
                            } else {
                                plugin.sendKeyPress(label, modifiers.shift, modifiers.ctrl, modifiers.alt)
                                if (modifiers.shift === 1) {
                                    modifiers.shift = 0;
                                }
                            }
                            if (modifiers.ctrl === 1) {
                                modifiers.ctrl = 0;
                            }
                            if (modifiers.alt === 1) {
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
                        width: page.width / _keyboardLayout.row2.length
                        height: page.height / 7
                        label: displayCorrectChar(modelData)

                        onClicked: {
                            if (page.chars) {
                                plugin.sendKeyPress(label, false, modifiers.ctrl, modifiers.alt)
                            } else {
                                plugin.sendKeyPress(label, modifiers.shift, modifiers.ctrl, modifiers.alt)
                                if (modifiers.shift === 1) {
                                    modifiers.shift = 0;
                                }
                            }
                            if (modifiers.ctrl === 1) {
                                modifiers.ctrl = 0;
                            }
                            if (modifiers.alt === 1) {
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
                        width: page.width / (_keyboardLayout.row3.length + (page.chars ? 0 : -1))
                        height: page.height / 7
                        label: getName(modelData)
                        markValue: modelData["caption"] === "shift" ? modifiers.shift : 0

                        function getName(data) {
                            if (modelData["caption"] === "shift") {
                                if (page.chars) {
                                    if (modifiers.shift === 0) {
                                        return "1/2";
                                    }
                                    return "2/2";
                                } else {
                                    return " ";
                                }
                            } else if (modelData["caption"] === "backspace") {
                                return " ";
                            } else {
                                return displayCorrectChar(data);
                            }
                        }

                        Image {
                            anchors.centerIn: parent
                            width: parent.width
                            height: parent.height
                            fillMode: Image.PreserveAspectFit
                            visible: (modelData["caption"] === "shift" && !page.chars) || modelData["caption"] === "backspace" || (modelData["symView"] === "up" && page.chars)

                            source: modelData["symView"] === "up" ? "image://theme/icon-m-up" : (modelData["caption"] === "shift" ? "image://theme/icon-m-capslock" : "image://theme/icon-m-backspace")
                        }

                        onClicked: {
                            if (modelData["caption"] === "shift") {
                                modifiers.shift++;
                                if (modifiers.shift > 2) {
                                    modifiers.shift = 0;
                                }
                                if (page.chars && modifiers.shift === 1) modifiers.shift = 2;
                                return;
                            } else if (modelData["caption"] === "backspace") {
                                plugin.sendKeyPress("backspace");
                            } else if (modelData["symView"] === "up") {
                                plugin.sendKeyPress("up", false, modifiers.ctrl, modifiers.alt);
                            } else {
                                if (page.chars) {
                                    plugin.sendKeyPress(label, false, modifiers.ctrl, modifiers.alt);
                                } else {
                                    plugin.sendKeyPress(label, modifiers.shift, modifiers.ctrl, modifiers.alt);
                                    if (modifiers.shift === 1) {
                                        modifiers.shift = 0;
                                    }
                                }
                                if (modifiers.ctrl === 1) {
                                    modifiers.ctrl = 0;
                                }
                                if (modifiers.alt === 1) {
                                    modifiers.alt = 0;
                                }
                            }
                        }
                    }
                }
            }

            Row {
                Repeater {
                    model: _keyboardLayout.row4

                    Key {
                        width: modelData["caption"] !== " " ? page.width / (_keyboardLayout.row4.length + 1) : 2 * page.width / (_keyboardLayout.row4.length + 1)
                        height: page.height / 7
                        label: displayCorrectChar(modelData)
                        markValue: modelData["caption"] === "ctrl" ? modifiers.ctrl : (modelData["caption"] === "alt" && !page.chars ? modifiers.alt : 0)

                        Image {
                            anchors.centerIn: parent
                            width: parent.width
                            height: parent.height
                            fillMode: Image.PreserveAspectFit
                            visible: (modelData["symView"] === "left" || modelData["symView"] === "down" || modelData["symView"] === "right") && page.chars

                            source:  "image://theme/icon-m-" + modelData["symView"]
                        }

                        onClicked: {
                            if (label == "?123" || label == "ABC") {
                                page.chars = !page.chars;
                                modifiers.shift = 0;
                                modifiers.alt = 0;
                                modifiers.ctrl = 0;
                            } else if (label == "alt") {
                                modifiers.alt++;
                                if (modifiers.alt > 2) modifiers.alt = 0;
                            } else if (label == "ctrl") {
                                modifiers.ctrl++;
                                if (modifiers.ctrl > 2) modifiers.ctrl = 0;
                            } else if (label == "enter") {
                                plugin.sendKeyPress("enter", modifiers.shift, modifiers.ctrl, modifiers.alt);
                            } else if (modelData["symView"] === "left") {
                                plugin.sendKeyPress("left", false, modifiers.ctrl, modifiers.alt);
                            } else if (modelData["symView"] === "down") {
                                plugin.sendKeyPress("down", false, modifiers.ctrl, modifiers.alt);
                            } else if (modelData["symView"] === "right") {
                                plugin.sendKeyPress("right", false, modifiers.ctrl, modifiers.alt);
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
