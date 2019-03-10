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
    allowedOrientations: Orientation.All
    property bool shiftActive: false
    property var device
    property var plugin: device.plugin("SailfishConnect::RemoteInputPlugin")

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
                        showAlt: shiftActive

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
                                plugin.sendKeyPress("backspace", shiftActive);
                            } else {
                                plugin.sendKeyPress(shiftActive ? altLabel : label, shiftActive);
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
                        showAlt: shiftActive

                        onClicked: {
                            plugin.sendKeyPress(shiftActive ? altLabel : label, shiftActive)
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
                        showAlt: shiftActive

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
                                plugin.sendKeyPress("enter", shiftActive);
                            } else {
                                plugin.sendKeyPress(shiftActive ? altLabel : label, shiftActive)
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
                        showAlt: shiftActive

                        onClicked: {
                            plugin.sendKeyPress(shiftActive ? altLabel : label)
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
                        showAlt: shiftActive
                        markable: label == "shift"

                        Image {
                            anchors.centerIn: parent
                            width: parent.width
                            height: parent.height
                            fillMode: Image.PreserveAspectFit
                            visible: modelData["name"] === "up"

                            source: "image://theme/icon-m-up"
                        }

                        Connections {
                            target: page

                            onShiftActiveChanged: {
                                if (!page.shiftActive && key.markValue == 1) {
                                    key.markValue = false;
                                }
                            }
                        }

                        onClicked: {
                            if (label === "shift") {
                                shiftActive = true;
                                markValue++;
                                if (markValue == 3) {
                                    markValue = 0;
                                    shiftActive = false
                                }
                            } else if (modelData["name"] === "up") {
                                plugin.sendKeyPress("up", shiftActive);
                            } else {
                                plugin.sendKeyPress(shiftActive ? altLabel : label, shiftActive)
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
                        showAlt: shiftActive

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
                                plugin.sendKeyPress("left", shiftActive);
                            } else if (modelData["name"] === "down") {
                                plugin.sendKeyPress("down", shiftActive);
                            } else if (modelData["name"] === "right") {
                                plugin.sendKeyPress("right", shiftActive);
                            } else {
                                plugin.sendKeyPress(label, shiftActive)
                            }
                        }
                    }
                }
            }
        }
    }
}
