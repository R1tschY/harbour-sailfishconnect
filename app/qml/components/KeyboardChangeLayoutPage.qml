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

    SilicaListView {
        id: view
        anchors.fill: parent

        header:  PageHeader {
            title: qsTr("Choose layout")
        }

        model: _keyboardLayout.layouts()

        delegate: BackgroundItem {
            width: view.width
            height: Theme.itemSizeSmall

            Text {
                anchors {
                    left: parent.left
                    leftMargin: Theme.horizontalPageMargin
                    right: parent.right
                    rightMargin: Theme.horizontalPageMargin
                    verticalCenter: parent.verticalCenter
                }
                text: modelData
                color: modelData == _keyboardLayout.layout ? Theme.highlightColor : Theme.secondaryColor
                font.pixelSize: Theme.fontSizeMedium
            }

            onClicked: {
                _keyboardLayout.layout = modelData;
            }
        }
    }
}
