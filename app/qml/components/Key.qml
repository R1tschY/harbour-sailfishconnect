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

Rectangle {
    radius: 10

    color: "transparent"
    border.color: Theme.primaryColor
    border.width: 2

    property string label: ""
    property int markValue: 0

    visible: label != ""

    signal clicked()

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        height: parent.height / 2 * markValue
        radius: 2

        color: Theme.secondaryHighlightColor
    }

    Text {
        anchors.centerIn: parent
        color: Theme.primaryColor
        font.pixelSize: Theme.fontSizeMedium

        text: label
    }

    Timer {
        id: timer

        interval: keyboardLayout.repeatInterval
        repeat: true
        running: false

        onTriggered: {
            if (keyboardLayout.feedback) {
                keyboardLayout.pressFeedback();
            }

            parent.clicked();
        }
    }

    MouseArea {
        id: mouse
        anchors.fill: parent

        onClicked: {
            if (keyboardLayout.feedback) {
                keyboardLayout.pressFeedback();
            }

            parent.clicked();
        }

        onPressAndHold: {
            timer.start();
        }

        onReleased: {
            if (keyboardLayout.feedback) {
                keyboardLayout.releaseFeedback();
            }

            timer.stop();
        }

        onEntered: {
            parent.color = Theme.highlightColor;
        }

        onExited: {
            parent.color = "transparent";
        }
    }
}
