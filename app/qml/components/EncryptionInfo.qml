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

Label {
    property string encryptionInfo

    text: prettifyEncryptionInfo(encryptionInfo)
    color: Theme.highlightColor
    wrapMode: Text.Wrap
    textFormat: Text.RichText

    function prettifyEncryptionInfo(encInfo) {
        function replacer(match) {
            var parts = match.split(":")
            var result = ""
            for (var i = 0; i < parts.length; i += 5) {
                result += parts.slice(i, i + 5).join(":") + "\n"
            }

            return "<pre style=\"text-align: center;\">" + result + "</pre><br>"
        }

        return encInfo.replace(/[a-f0-9]{2}(:[a-f0-9]{2})+/g, replacer)
    }
}
