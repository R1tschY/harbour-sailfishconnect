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

CoverBackground {
    id: cover

    Image {
        source: "image://theme/icon-l-tether"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: Theme.paddingLarge
        fillMode: Image.PreserveAspectFit
        sourceSize.width: Theme.iconSizeExtraLarge * 2
        sourceSize.height: Theme.iconSizeExtraLarge * 2
        opacity: 0.1
    }

    Component {
        id: deviceDelegate

        ListItem {
            id: listItem

            width: cover.width
            height: Theme.itemSizeExtraSmall * 0.75

            Label {
                id: deviceNameLabel
                anchors.verticalCenter: parent.verticalCenter
                x: Theme.paddingLarge

                text: name
                color: Theme.primaryColor
                truncationMode: TruncationMode.Fade
                textFormat: Text.PlainText
                font.pixelSize: Theme.fontSizeSmall
            }
        }
    }

    DeviceListModel {
        id: devices
    }

    SortFilterModel {
        id: reachableDevices

        sortRole: "name"
        filterRole: "reachable"
        filterValue: true
        sourceModel: devices
    }

    SilicaListView {
        anchors.fill: parent

        width: cover.width
        height: contentItem.childrenRect.height
        model: reachableDevices
        delegate: deviceDelegate
    }

    CoverActionList {
        id: coverAction

        CoverAction {
            iconSource: "image://theme/icon-cover-refresh"
            onTriggered: daemon.forceOnNetworkChange()
        }
    }
}

