/*
 * Copyright 2015 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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

import QtQuick 2.6
import Sailfish.Silica 1.0
import SailfishConnect.Core 0.2
import SailfishConnect.RemoteControl 0.2

Page {
    id: page

    property var device
    property var plugin:
        device.plugin("SailfishConnect::RemoteControlPlugin")
    property int lastX: 0xDEAD
    property int lastY: 0xDEAD
    property bool moved: false

    PageHeader {
        id: header
        title: qsTr("Touchpad")
    }

    InfoLabel {
        id: hintLabel
        text: qsTr("")
    }

    MouseArea {
        id: touchpad        
        anchors {
            top: header.bottom
            left: page.left
            right: page.right
            bottom: page.bottom
        }

        hoverEnabled: true
        scrollGestureEnabled: true
        propagateComposedEvents: false
        preventStealing: true
        acceptedButtons: Qt.AllButtons  // FIXME: is ignored

        onPositionChanged: {
            if (pressed) {
                moved = true
                if (lastX !== 0xDEAD) {
                    plugin.move(mouse.x - lastX, mouse.y - lastY)
                }

                lastX = mouse.x
                lastY = mouse.y
            }
        }

        onReleased: {
            lastX = lastY = 0xDEAD
        }

        onClicked: {
            if (!moved) {
                plugin.sendCommand("singleclick", true)
            }
            moved = false
        }

        onDoubleClicked: {
            if (!moved) {
                plugin.sendCommand("doubleclick", true)
            }
            moved = false
        }

        onPressAndHold: {
            if (!moved) {
                plugin.sendCommand("singlehold", true)
            }
            moved = false
        }

        onWheel: {
            // FIXME: is ignored
            plugin.scroll(
                wheel.angleDelta.x / 120,
                wheel.angleDelta.y / 120)
        }
    }
}
