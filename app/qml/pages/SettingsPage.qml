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
import SailfishConnect.Api 0.6
import SailfishConnect.UI 0.6

Page {
    id: page

    allowedOrientations: Orientation.All

    SilicaFlickable {
        id: pageFlickable
        anchors.fill: parent
        contentHeight: content.height

        Column {
            id: content

            width: page.width

            PageHeader {
                title: i18n("Settings")
            }

            TextSwitch {
                id: runInBackground
                text: i18n("Run as background service")
                description:
                    i18n("Application starts at system start and runs in background.")
                checked: ui.runInBackground
                onClicked: ui.runInBackground = !checked
                automaticCheck: false
            }

            SectionHeader {
                text: i18n("Devices from IP address")
            }
            ColumnView {
                id: customDevicesView
                model: SortFilterModel {
                    id: sortedCustomDevicesModel
                    sortRole: "stringData"
                    filterMode: "none"
                    sourceModel: StringListModel {
                        id: customDevicesModel
                        onStringListChanged: daemon.customDevices = stringList
                        Component.onCompleted: stringList = daemon.customDevices
                    }
                }

                itemHeight: Theme.itemSizeExtraSmall

                delegate: ListItem {
                    id: listItem
                    contentHeight: Theme.itemSizeExtraSmall

                    menu: ContextMenu {
                        MenuItem {
                            text: i18n("Edit")
                            onClicked: {
                                var dialog = pageStack.push(
                                    Qt.resolvedUrl("AddCustomDeviceDialog.qml"),
                                    { "key": display, "address": display })
                                dialog.accepted.connect(function() {
                                    display = dialog.address
                                })
                            }
                        }

                        MenuItem {
                            text: i18n("Remove")
                            onClicked: customDevicesModel.removeOne(display)
                        }
                    }

                    Label {
                        x: Theme.horizontalPageMargin
                        y: Theme.paddingMedium
                        width: page.width - 2 * Theme.horizontalPageMargin
                        height: Theme.itemSizeExtraSmall
                        text: display
                        font.pixelSize: Theme.fontSizeMedium
                    }
                }
            }

            Label {
                id: replacement

                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                height: Theme.itemSizeExtraSmall

                visible: customDevicesView.count === 0
                opacity: visible ? 1.0 : 0

                text: i18n("No devices defined")
                horizontalAlignment: Text.AlignHCenter
                font {
                    pixelSize: Theme.fontSizeMedium
                    family: Theme.fontFamilyHeading
                }
                color: Theme.rgba(Theme.highlightColor, 0.6)

                Behavior on opacity { FadeAnimation { duration: 300 } }
            }
        }

        PullDownMenu {
            MenuItem {
                text: i18n("Add device from IP address")
                onClicked: {
                    var dialog = pageStack.push(Qt.resolvedUrl("AddCustomDeviceDialog.qml"))
                    dialog.accepted.connect(function() {
                        customDevicesModel.append(dialog.address)
                    })
                }
            }
        }

        VerticalScrollDecorator { flickable: pageFlickable }
    }
}
