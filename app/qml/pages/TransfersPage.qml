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

import QtQuick 2.0
import Sailfish.Silica 1.0
import SailfishConnect.UI 0.3

Page {

    allowedOrientations: Orientation.All

    SilicaListView {
        id: listView
        model: JobsModel { }
        anchors.fill: parent
        header: PageHeader {
            title: qsTr("Transfers")
        }

        delegate: ListItem {
            id: listItem

            width: page.width
            height: Theme.itemSizeMedium

            ProgressCircle {
                id: progress

                inAlternateCycle: true

                x: Theme.horizontalPageMargin
                anchors.verticalCenter: parent.verticalCenter
                width: Theme.iconSizeMedium
                height: Theme.iconSizeMedium

                value: processedBytes * 100.0 / totalBytes

                Image {
                    source: "image://theme/icon-s-device-download"
                    anchors { centerIn: progress }
                }
            }

            Label {
                id: jobTitle
                anchors {
                    left: icon.right
                    leftMargin: Theme.paddingLarge
                    right: parent.right
                    rightMargin: Theme.horizontalPageMargin
                }
                y: listItem.contentHeight / 2 - implicitHeight / 2
                   - (deviceStatusLabel.implicitHeight / 2)

                text: name
                color: listItem.highlighted
                       ? Theme.highlightColor
                       : Theme.primaryColor
                truncationMode: TruncationMode.Fade
                textFormat: Text.PlainText
            }

            Label {
                id: jobDescription
                anchors {
                    left: jobTitle.left
                    top: jobTitle.bottom
                    right: parent.right
                    rightMargin: Theme.horizontalPageMargin
                }

                text: description
                color: listItem.highlighted
                       ? Theme.secondaryHighlightColor
                       : Theme.secondaryColor
                truncationMode: TruncationMode.Fade
                font.pixelSize: Theme.fontSizeExtraSmall
                width: parent.width
                textFormat: Text.PlainText
            }
        }

        VerticalScrollDecorator { flickable: listView }
    }

}
