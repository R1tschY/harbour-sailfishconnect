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


Page {
    id: page

    property QtObject device

    allowedOrientations: Orientation.All

    function prettifyEncryptionInfo(encInfo) {
        var re = /\w\w(:\w\w)+/g

        function replacer(match) {
            var parts = match.split(":")
            var result = ""
            for (var i = 0; i < parts.length; i += 5) {
                result += "  " + parts[i + 0] + ":" + parts[i + 1] + ":"
                        + parts[i + 2] + ":" + parts[i + 3] + ":" + parts[i + 4]
                        + "\n"
            }

            return "<br><pre>" + result + "</pre><br>"
        }

        return encInfo.replace(re, replacer)
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: content.height

        Column {
            id: content

            width: page.width
            spacing: Theme.paddingLarge

            PageHeader {
                id: header
                title: i18n("Encryption info")
            }

            Label {
                x: Theme.horizontalPageMargin
                width: page.width - 2 * Theme.horizontalPageMargin

                text: device ? prettifyEncryptionInfo(device.encryptionInfo()) : ""
                color: Theme.highlightColor
                wrapMode: Text.Wrap
                textFormat: Text.StyledText
            }
        }

        VerticalScrollDecorator {}
    }
}
