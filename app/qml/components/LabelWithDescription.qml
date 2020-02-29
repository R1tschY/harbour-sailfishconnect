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

import QtQuick 2.0
import Sailfish.Silica 1.0

Column {
    property bool highlighted: false
    property alias title: mainLabel.text
    property alias description: descripionLabel.text

    Label {
        id: mainLabel
        width: parent.width
        
        color: highlighted
                ? Theme.highlightColor
                : Theme.primaryColor
        truncationMode: TruncationMode.Fade
        textFormat: Text.PlainText
    }
    Label {
        id: descripionLabel
        width: parent.width

        color: highlighted
                ? Theme.secondaryHighlightColor
                : Theme.secondaryColor
        font.pixelSize: Theme.fontSizeExtraSmall
        truncationMode: TruncationMode.Fade
        textFormat: Text.PlainText
    }
}