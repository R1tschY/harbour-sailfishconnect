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
import "../components"

Page {
    id: page
    allowedOrientations: Orientation.All
    property QtObject device
    property QtObject plugin: device.plugin("SailfishConnect::RemoteKeyboardPlugin")
    property QtObject modifiers: QtObject {
        property int shift: 0
        property int ctrl: 0
        property int alt: 0
    }
    property bool chars: false

    function displayCorrectChar(data) {
        if (page.chars && typeof data["symView"] !== "undefined") {
            // dont show text on arrow keys
            if (["up", "down", "left", "right"].indexOf(data["symView"]) !== -1) return " "
            if (modifiers.shift && typeof data["symView2"] !== "undefined") {
                return data["symView2"]
            }
            return data["symView"]
        } else {
            if (modifiers.shift && typeof data["captionShifted"] !== "undefined") {
                return data["captionShifted"]
            }
            return data["caption"]
        }
    }

    SilicaFlickable {
        anchors.fill: parent

        PageHeader {
            title: i18n("Keyboard")
        }

        PullDownMenu {
            MenuItem {
                text: i18n("Settings")
                onClicked: {
                    pageStack.push(Qt.resolvedUrl("KeyboardChangeLayoutPage.qml"))
                }
            }
        }

        Column {
            anchors.bottom: parent.bottom

            Row {
                Repeater {
                    model: keyboardLayout.row1

                    Key {
                        width: page.width / keyboardLayout.row1.length
                        height: page.height / 7
                        label: displayCorrectChar(modelData)

                        onClicked: {
                            if (page.chars) {
                                plugin.sendKeyPress(label, false, modifiers.ctrl, modifiers.alt)
                            } else {
                                plugin.sendKeyPress(label, modifiers.shift, modifiers.ctrl, modifiers.alt)
                                if (modifiers.shift === 1) {
                                    modifiers.shift = 0
                                }
                            }
                            if (modifiers.ctrl === 1) {
                                modifiers.ctrl = 0
                            }
                            if (modifiers.alt === 1) {
                                modifiers.alt = 0
                            }
                        }
                    }
                }
            }

            Row {
                Repeater {
                    model: keyboardLayout.row2

                    Key {
                        width: page.width / keyboardLayout.row2.length
                        height: page.height / 7
                        label: displayCorrectChar(modelData)

                        onClicked: {
                            if (page.chars) {
                                plugin.sendKeyPress(label, false, modifiers.ctrl, modifiers.alt)
                            } else {
                                plugin.sendKeyPress(label, modifiers.shift, modifiers.ctrl, modifiers.alt)
                                if (modifiers.shift === 1) {
                                    modifiers.shift = 0
                                }
                            }
                            if (modifiers.ctrl === 1) {
                                modifiers.ctrl = 0
                            }
                            if (modifiers.alt === 1) {
                                modifiers.alt = 0
                            }
                        }
                    }
                }
            }

            Row {
                Repeater {
                    model: keyboardLayout.row3

                    Key {
                        width: page.width / keyboardLayout.row3.length
                        height: page.height / 7
                        label: displayCorrectChar(modelData)

                        onClicked: {
                            if (page.chars) {
                                plugin.sendKeyPress(label, false, modifiers.ctrl, modifiers.alt)
                            } else {
                                plugin.sendKeyPress(label, modifiers.shift, modifiers.ctrl, modifiers.alt)
                                if (modifiers.shift === 1) {
                                    modifiers.shift = 0
                                }
                            }
                            if (modifiers.ctrl === 1) {
                                modifiers.ctrl = 0
                            }
                            if (modifiers.alt === 1) {
                                modifiers.alt = 0
                            }
                        }
                    }
                }
            }

            Row {
                Repeater {
                    model: keyboardLayout.row4

                    Key {
                        width: page.width / (keyboardLayout.row4.length + (page.chars ? 0 : -1))
                        height: page.height / 7
                        label: getName(modelData)
                        markValue: modelData["caption"] === "shift" ? modifiers.shift : 0

                        function getName(data) {
                            if (modelData["caption"] === "shift") {
                                if (page.chars) {
                                    if (modifiers.shift === 0) {
                                        return "1/2"
                                    }
                                    return "2/2"
                                } else {
                                    return " "
                                }
                            } else if (modelData["caption"] === "backspace") {
                                return " "
                            } else {
                                return displayCorrectChar(data)
                            }
                        }

                        Image {
                            anchors.centerIn: parent
                            width: parent.width
                            height: parent.height
                            fillMode: Image.PreserveAspectFit
                            visible: (modelData["caption"] === "shift" && !page.chars) ||
                                     modelData["caption"] === "backspace" ||
                                     (modelData["symView"] === "up" && page.chars)

                            source: modelData["symView"] === "up" ?
                                        "image://theme/icon-m-up" :
                                        (modelData["caption"] === "shift" ?
                                             "image://theme/icon-m-capslock" :
                                             "image://theme/icon-m-backspace")
                        }

                        onClicked: {
                            if (modelData["caption"] === "shift") {
                                modifiers.shift = (modifiers.shift + 1) % 3
                                if (page.chars && modifiers.shift === 1) modifiers.shift = 2
                                return
                            } else if (modelData["caption"] === "backspace") {
                                plugin.sendKeyPress("backspace")
                            } else if (modelData["symView"] === "up") {
                                plugin.sendKeyPress("up", false, modifiers.ctrl, modifiers.alt)
                            } else {
                                if (page.chars) {
                                    plugin.sendKeyPress(label, false, modifiers.ctrl, modifiers.alt)
                                } else {
                                    plugin.sendKeyPress(label, modifiers.shift, modifiers.ctrl, modifiers.alt)
                                    if (modifiers.shift === 1) {
                                        modifiers.shift = 0
                                    }
                                }
                                if (modifiers.ctrl === 1) {
                                    modifiers.ctrl = 0
                                }
                                if (modifiers.alt === 1) {
                                    modifiers.alt = 0
                                }
                            }
                        }
                    }
                }
            }

            Row {
                Repeater {
                    model: keyboardLayout.row5

                    Key {
                        width: modelData["caption"] !== " " ?
                                   page.width / (keyboardLayout.row5.length + 1)
                                   : 2 * page.width / (keyboardLayout.row5.length + 1)
                        height: page.height / 7
                        label: displayCorrectChar(modelData)
                        markValue: modelData["caption"] === "ctrl" ?
                                       modifiers.ctrl
                                       : (modelData["caption"] === "alt" && !page.chars ?
                                            modifiers.alt : 0)

                        Image {
                            anchors.centerIn: parent
                            width: parent.width
                            height: parent.height
                            fillMode: Image.PreserveAspectFit
                            visible: (["left", "right", "down"].indexOf(modelData["symView"]) !== -1) && page.chars

                            source: visible ? "image://theme/icon-m-" + modelData["symView"] : ""
                        }

                        onClicked: {
                            if (label === "?123" || label === "ABC") {
                                page.chars = !page.chars
                                modifiers.shift = 0
                                modifiers.alt = 0
                                modifiers.ctrl = 0
                            } else if (label === "alt") {
                                modifiers.alt = (modifiers.alt + 1) % 3
                            } else if (label === "ctrl") {
                                modifiers.ctrl = (modifiers.ctrl + 1) % 3
                            } else if (label === "enter") {
                                plugin.sendKeyPress("enter", modifiers.shift, modifiers.ctrl, modifiers.alt)
                            } else if (modelData["symView"] === "left") {
                                plugin.sendKeyPress("left", false, modifiers.ctrl, modifiers.alt)
                            } else if (modelData["symView"] === "down") {
                                plugin.sendKeyPress("down", false, modifiers.ctrl, modifiers.alt)
                            } else if (modelData["symView"] === "right") {
                                plugin.sendKeyPress("right", false, modifiers.ctrl, modifiers.alt)
                            }
                        }
                    }
                }
            }
        }
    }
}
