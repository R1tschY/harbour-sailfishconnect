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
import SailfishConnect.UI 0.6
import org.kde.kdeconnect 1.0

Page {
    id: page

    allowedOrientations: Orientation.All

    property QtObject device
    property QtObject plugin: device.getRemoteCommandsApi()

    SilicaListView {
        id: view
        anchors.fill: parent

        header: PageHeader {
            title: i18n("Run command")
        }

        model: SortFilterModel {
            sortRole: "name"
            sourceModel: RemoteCommandsModel {
                deviceId: device.id()
            }
        }

        delegate: ListItem {
            id: listItem
            width: page.width
            height: Theme.itemSizeMedium

            Column {
                width: page.width - 2 * Theme.horizontalPageMargin
                x: Theme.horizontalPageMargin

                Label {
                    width: parent.width
                    
                    text: name
                    
                    color: listItem.highlighted
                            ? Theme.highlightColor
                            : Theme.primaryColor
                    truncationMode: TruncationMode.Fade
                    textFormat: Text.PlainText
                }
                Label {
                    width: parent.width

                    text: command
                    
                    color: listItem.highlighted
                            ? Theme.secondaryHighlightColor
                            : Theme.secondaryColor
                    font.pixelSize: Theme.fontSizeExtraSmall
                    truncationMode: TruncationMode.Fade
                    textFormat: Text.PlainText
                }
            }

            onClicked: plugin.triggerCommand(key)
        }

        ViewPlaceholder {
            id: placeholder
            enabled: view.count === 0
            flickable: view
            text: i18n("No commands defined on other device")
        }

        VerticalScrollDecorator { flickable: view }
    }
}
