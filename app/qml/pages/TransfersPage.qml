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
import Nemo.Notifications 1.0
import Sailfish.Silica 1.0
import SailfishConnect.UI 0.3
import SailfishConnect.Core 0.3
import SailfishConnect.Qml 0.4

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

    function openFile(target) {
        var url = Url.fromUrl(target)
        if (url.scheme !== "local") {
            console.error("Tried to open non-local file")
            return
        }

        if (xdgOpen.state === "Running") {
            xdgOpen.waitForFinished()
        }

        xdgOpen.arguments = [url.path]
        xdgOpen.start()
        // TODO: start loading animation
    }

//    function openFileDir(target) {
//        var url = Url.fromUrl(target)
//        if (url.scheme !== "local") {
//            console.error("Tried to open non-local file")
//            return
//        }

//        if (xdgOpen.state == "Running") {
//            xdgOpen.waitForFinished()
//        }

//        xdgOpen.arguments = [Path.dirname(url.path)]
//        xdgOpen.start()
//        // TODO: start loading animation
//    }

    function progressIcon(target) {
        if (stringStartsWith(target.toString(), "remote:")) {
            return "image://theme/icon-s-device-download"
        } else {
            return "image://theme/icon-s-device-upload"
        }
    }

    function description(currentState, deviceId, processedBytes,
                         totalBytes, error) {
        var deviceName = ""
        var device = deviceId ? daemon.getDevice(deviceId) : null
        if (device) {
            deviceName = device.name
        }

        if (error) {
            return "%1 - %2"
                .arg(qsTr("Failed"))
                .arg(deviceName)
        }

        if (currentState === "running") {
            if (totalBytes < 0) {
                return "%1 - %2"
                    .arg(deviceName)
                    .arg(Humanize.bytes(processedBytes))
            }

            return "%1 - %2 %3 %4"
                .arg(deviceName)
                .arg(Humanize.bytes(processedBytes))
                //: Download progress, for example: 3MB of 50MB
                .arg(qsTr("of"))
                .arg(Humanize.bytes(totalBytes))
        } else if (currentState === "finished") {
            return "%1 - %2 - %3"
                .arg(qsTr("Completed"))
                .arg(deviceName)
                .arg(Humanize.bytes(processedBytes))
        } else if (currentState === "canceled") {
            return "%1 - %2"
                .arg(qsTr("Canceled"))
                .arg(deviceName)
        }
    }

    Notification { id: xdgOpenNotification }

    Process {
        id: xdgOpen

        program: "xdg-open"

        onFinished: {
            console.log("onFinished")
            if (!xdgOpen.normalExit || xdgOpen.exitCode !== 0) {
                xdgOpenNotification.previewSummary = qsTr("Failed to open file")
                if (xdgOpen.normalExit) {
                    var errorMessage = {
                        // Error in command line syntax.
                        1: qsTr("Internal error"),

                        // One of the files passed on the command line did not exist.
                        2: qsTr("File does not exit"),

                        // A required tool could not be found.
                        3: qsTr("No program found to open file"),

                        // The action failed.
                        4: qsTr("xdg-open failed"),
                    }

                    xdgOpenNotification.previewBody = errorMessage[xdgOpen.exitCode]
                } else {
                    xdgOpenNotification.previewBody = qsTr("xdg-open crashed")
                }
                xdgOpenNotification.publish()
            }
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

            property bool isTargetLocal:
                stringStartsWith(target.toString(), "local:")
            property bool wasSuccessful: currentState === "finished" && !error

            width: page.width
            contentHeight: Theme.itemSizeMedium

            menu: ContextMenu {
                MenuItem {
                    text: qsTr("Cancel")
                    visible: currentState === "running"
                    onClicked: canceled = true
                }
                MenuItem {
                    text: qsTr("Open")
                    visible: isTargetLocal && wasSuccessful
                    onClicked: openFile(target)
                }
//                TODO: MenuItem {
//                    text: qsTr("Open containing directory")
//                    visible: isTargetLocal && wasSuccessful
//                    onClicked: openFileDir(target)
//                }
            }

            ProgressCircle {
                id: progress

                // inAlternateCycle: true

                x: Theme.horizontalPageMargin
                anchors.verticalCenter: parent.verticalCenter
                width: Theme.iconSizeMedium
                height: Theme.iconSizeMedium
                visible: currentState === "running" && totalBytes > 0

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
                visible: wasSuccessful
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

                text: description(currentState, deviceId,
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
            enabled: listView.count === 0
            text: qsTr("Yet no downloads or uploads")
        }

        VerticalScrollDecorator { flickable: listView }
    }

}
