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
    allowedOrientations: Orientation.All

    SilicaFlickable {
        anchors.fill: parent

        ListView {
            id: view
            anchors.fill: parent

            model: _keyboardLayout.layouts()

            header: PageHeader {
                title: qsTr("Choose layout")
            }

            delegate: BackgroundItem {
                width: view.width
                height: Theme.itemSizeSmall

                Text {
                    anchors {
                        left: parent.left
                        leftMargin: Theme.horizontalPageMargin
                        right: parent.right
                        rightMargin: Theme.horizontalPageMargin
                        verticalCenter: parent.verticalCenter
                    }
                    text: modelData
                    color: modelData == _keyboardLayout.layout ? Theme.highlightColor : Theme.secondaryColor
                    font.pixelSize: Theme.fontSizeMedium
                }

                onClicked: {
                    _keyboardLayout.layout = modelData;
                }
            }

            footer: Column {

                Slider {
                    width: view.width
                    minimumValue: 50
                    maximumValue: 500
                    stepSize: 50
                    value: _keyboardLayout.repeatInterval
                    valueText: qsTr("%1ms repeat interval").arg(value)

                    onValueChanged: {
                        _keyboardLayout.repeatInterval = value;
                    }
                }

                TextSwitch {
                    text: qsTr("Vibration feedback")
                    checked: _keyboardLayout.feedback

                    onCheckedChanged: {
                        _keyboardLayout.feedback = checked;
                    }
                }
            }
        }
    }
}
