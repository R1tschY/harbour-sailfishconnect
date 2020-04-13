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
import SailfishConnect.Api 0.7


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
            spacing: Theme.paddingLarge

            PageHeader {
                title: i18n("Settings")
            }

            TextSwitch {
                id: runInBackground
                text: i18n("Run as background service")
                description:
                    i18n("Application starts at system start and runs in " +
                         "background.")
                checked: ui.runInBackground
                onClicked: ui.runInBackground = !checked
                automaticCheck: false
            }
        }

        VerticalScrollDecorator { flickable: pageFlickable }
    }
}
