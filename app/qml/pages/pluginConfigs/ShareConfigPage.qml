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

Page {
    id: page

    allowedOrientations: Orientation.All

    property QtObject device
    property QtObject config: device.getPluginConfig("sailfishconnect_share")

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: content.height

        Column {
            id: content

            width: page.width

            PageHeader {
                id: header
                title: i18n("Edit share configuration")
            }

            TextField {
                id: downloadFolderField
                width: parent.width
                label: i18n("Folder to save files")
                text: config.get("incoming_path")

                onActiveFocusChanged: {
                    if (activeFocus)
                        return

                    if (text.length === 0) {
                        text = config.get("incoming_path")
                    } else {
                        config.set("incoming_path", text)
                        placeholderText = config.get("incoming_path")
                    }
                }

                EnterKey.onClicked: downloadFolderField.focus = false
                EnterKey.iconSource: "image://theme/icon-m-enter-close"
            }

            Label {
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.WordWrap
                color: Theme.secondaryColor
                textFormat: Text.RichText
                x: Theme.horizontalPageMargin
                text: i18n("&percnt;1 in the path will be replaced with the specific device name.")
            }
        }

        VerticalScrollDecorator { }
    }
}
