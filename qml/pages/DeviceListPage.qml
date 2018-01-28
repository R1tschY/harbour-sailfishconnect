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
import Nemo.Notifications 1.0
import SailfishConnect 0.1

Page {
    id: page

    function selectDeviceColor(trusted, reachable) {
        if (trusted) {
            return (reachable
                ? Theme.highlightColor
                : Theme.secondaryHighlightColor)
        } else {
            return (reachable
                ? Theme.primaryColor
                : Theme.secondaryColor)
        }
    }

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.Portrait

    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaFlickable {
        anchors.fill: parent

//        // PullDownMenu and PushUpMenu must be declared in SilicaFlickable, SilicaListView or SilicaGridView
//        PullDownMenu {
//            MenuItem {
//                text: qsTr("Show Page 2")
//                onClicked: pageStack.push(Qt.resolvedUrl("SecondPage.qml"))
//            }
//        }

        // Tell SilicaFlickable the height of its content.
        contentHeight: column.height

        // Place our content in a Column.  The PageHeader is always placed at the top
        // of the page, followed by our content.
        Column {
            id: column

            width: page.width
            spacing: Theme.paddingLarge
            PageHeader {
                title: qsTr("Select device")
            }

//            Button {
//                Notification {
//                    id: notification
//                    category: "x-nemo.example"
//                    summary: "Notification summary"
//                    body: "Notification body"
//                    onClicked: console.log("Clicked")
//                }
//                text: "Application notification" + (notification.replacesId ? " ID:" + notification.replacesId : "")
//                onClicked: notification.publish()
//            }

            Component {
                id: deviceDelegate

                Item {
                    width: page.width
                    height: Theme.fontSizeMedium + Theme.fontSizeSmall + Theme.paddingSmall * 4
                    Row {
                        Image {
                            height: Theme.fontSizeMedium + Theme.fontSizeSmall
                            width: Theme.fontSizeMedium + Theme.fontSizeSmall
                            source: iconUrl + "?" +
                                    selectDeviceColor(trusted, reachable)
                        }
                        Column {
                            Text {
                                font.pixelSize: Theme.fontSizeMedium
                                text: '<b>Name:</b> ' + name
                                color: "white"
                            }
                            Text {
                                font.pixelSize: Theme.fontSizeSmall
                                text: '<b>Id:</b> ' + id
                                color: "white"
                            }
                        }
                    }
                }
            }

            SilicaListView {
                width: page.width; height: 200

                model: DeviceListModel { id: listModel }
                delegate: deviceDelegate
            }
        }
    }
}

