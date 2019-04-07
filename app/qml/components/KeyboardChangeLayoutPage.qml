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

        Column {
            anchors.fill: parent

            PageHeader {
                title: qsTr("Choose layout")
            }

            Slider {
                width: parent.width
                minimumValue: 50
                maximumValue: 500
                stepSize: 50
                value: _keyboardLayout.repeatInterval
                valueText: qsTr("%1ms repeat interval").arg(value)

                onValueChanged: {
                    if (_keyboardLayout.repeatInterval != value) _keyboardLayout.repeatInterval = value;
                }
            }

            TextSwitch {
                text: qsTr("Vibration feedback")
                checked: _keyboardLayout.feedback

                onCheckedChanged: {
                    if (_keyboardLayout.feedback != checked) _keyboardLayout.feedback = checked;
                }
            }

            ComboBox {
                label: qsTr("Change layout")
                currentIndex: _keyboardLayout.layouts().indexOf(_keyboardLayout.layout)
                menu: ContextMenu {
                    Repeater {
                        id: view
                        model: _keyboardLayout.layouts()

                        delegate: MenuItem {
                            width: view.width
                            height: Theme.itemSizeSmall
                            text: modelData

                            onClicked: {
                                _keyboardLayout.layout = modelData;
                            }
                        }
                    }
                }
            }
        }
    }
}
