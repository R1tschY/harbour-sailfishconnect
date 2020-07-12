/*
 * Copyright 2020 Richard Liebscher <richard.liebscher@gmail.com>.
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
import SailfishConnect.Api 0.6
import "../../components"

Dialog {
    id: page

    property string key: ""
    property alias name: nameField.text
    property alias command: commandField.text

    allowedOrientations: Orientation.All

    canAccept: !!name && !!command

    // https://bugreports.qt.io/browse/QTBUG-16289
    property var _predefined: [
        {
            "name": i18n("Reboot"),
            "command": "dsmetool --reboot"
        },
        {
            "name": i18n("Lock device"),
            "command": ("dbus-send --system --type=method_call \\\n" +
               "    --dest=org.nemomobile.devicelock /devicelock \\\n" +
               "    org.nemomobile.lipstick.devicelock.setState int32:1")
        }
    ]

    ListModel {
        id: predefinedModel

        Component.onCompleted: {
            for (var command in _predefined) {
                append(_predefined[command])
            }
        }
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: mainColumn.height

        Column {
            id: mainColumn
            width: parent.width

            DialogHeader { }

            TextField {
                id: nameField
                width: parent.width
                placeholderText: i18n("Name of command")
                label: i18n("Name")
                focus: true
            }

            TextArea {
                id: commandField
                width: parent.width
                placeholderText: i18n("Shell command to execute")
                label: i18n("Command")
                inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
            }

            SectionHeader {
                text: i18n("Predefined commands")
                visible: predefinedView.count > 0
            }

            ColumnView {
                id: predefinedView
                width: parent.width
                itemHeight: Theme.itemSizeMedium

                model: predefinedModel

                delegate: ListItem {
                    id: listItem
                    width: page.width
                    contentHeight: item.height

                    LabelWithDescription {
                        id: item
                        width: page.width - 2 * Theme.horizontalPageMargin
                        x: Theme.horizontalPageMargin
                        
                        title: name
                        description: command

                        highlighted: listItem.highlighted
                    }

                    onClicked: {
                        nameField.text = name
                        commandField.text = command
                    }
                }
            }
        }
    }
}
