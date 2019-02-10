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
import SailfishConnect.Core 0.3
import SailfishConnect.Qml 0.3

Page {
    id: page

    allowedOrientations: Orientation.All

    function stringStartsWith(s, start) {
        // TODO: Qt 5.8: use s.startsWith(start)
        return s.substring(0, start.length) === start
    }

    function targetFilename(target) {
        var url = Url.fromUrl(target)
        if (url.scheme === "local") {
            return url.fileName
        } else if (url.scheme === "remote") {
            return url.path
        } else {
            return url.toString()
        }
    }

    function progressIcon(target) {
        if (stringStartsWith(target.toString(), "remote:")) {
            return "image://theme/icon-s-device-download"
        } else {
            return "image://theme/icon-s-device-upload"
        }
    }

    function description(currentState, deviceId, action, processedBytes,
                         totalBytes, error) {
        var deviceName = "¯\_(ツ)_/¯"
        var device = daemon.getDevice(deviceId)
        if (device) {
            deviceName = device.name
        }

        if (currentState === Job.Running) {
            if (totalBytes < 0) {
                return "%1 - %2 - %3"
                    .arg(action)
                    .arg(deviceName)
                    .arg(Humanize.bytes(processedBytes))
            }

            return "%1 - %2 - %3 %4 %5"
                .arg(action)
                .arg(deviceName)
                .arg(Humanize.bytes(processedBytes))
                //: Download progress, for example: 3MB of 50MB
                .arg(qsTr("of"))
                .arg(Humanize.bytes(totalBytes))
        } else if (currentState === Job.Finished) {
            if (error) {
                return action
            }

            return "%1 - %2 - %3"
                .arg(action)
                .arg(deviceName)
                .arg(Humanize.bytes(processedBytes))
        } else if (currentState === Job.Pending) {
            if (totalBytes < 0) {
                return action
            }

            return "%1 - %2 - %3"
                .arg(action)
                .arg(deviceName)
                .arg(Humanize.bytes(totalBytes))
        }
    }

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

                // inAlternateCycle: true

                x: Theme.horizontalPageMargin
                anchors.verticalCenter: parent.verticalCenter
                width: Theme.iconSizeMedium
                height: Theme.iconSizeMedium
                visible: currentState === Job.Running && totalBytes > 0

                value: totalBytes > 0 ? (processedBytes / totalBytes) : 0
            }

            Image {
                source: progressIcon(target)
                anchors { centerIn: progress }
            }

            /* TODO Image {
                source: "image://theme/icon-l-image"
                anchors { centerIn: progress }
                opacity: 0.3
                visible: currentState === Job.Finished
            }*/

            Label {
                id: jobTarget
                anchors {
                    left: parent.left
                    leftMargin: Theme.horizontalPageMargin
                                + Theme.iconSizeMedium + Theme.paddingMedium
                    right: parent.right
                    top: parent.top
                }

                text: targetFilename(target)
                color: listItem.highlighted
                       ? Theme.highlightColor
                       : Theme.primaryColor
                width: parent.width - Theme.iconSizeMedium
                truncationMode: TruncationMode.Fade
                textFormat: Text.PlainText
            }

            Label {
                id: jobAction
                anchors {
                    left: jobTarget.left
                    top: jobTarget.bottom
                    right: parent.right
                }

                text: description(currentState, deviceId, action,
                                  processedBytes, totalBytes, error)
                color: listItem.highlighted
                       ? Theme.secondaryHighlightColor
                       : Theme.secondaryColor
                truncationMode: TruncationMode.Fade
                font.pixelSize: Theme.fontSizeExtraSmall
                width: parent.width - Theme.iconSizeMedium
                textFormat: Text.PlainText
            }
        }

        ViewPlaceholder {
            enabled: listView.count == 0
            text: qsTr("Yet no downloads or uploads")
        }

        VerticalScrollDecorator { flickable: listView }
    }

}
