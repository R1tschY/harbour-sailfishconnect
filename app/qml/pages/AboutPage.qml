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


Page {
    id: page

    allowedOrientations: Orientation.All

    SilicaFlickable {
        id: flickable
        anchors.fill: parent
        contentHeight: content.height

        Column {
            id: content

            width: page.width
            spacing: Theme.paddingLarge

            PageHeader {
                title: qsTr("About")
            }

            Label {
                text: "<b>Sailfish Connect</b>" // TODO: QT5.9: Qt.application.displayName

                wrapMode: Text.Wrap
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                horizontalAlignment: Text.AlignHCenter

                color: Theme.highlightColor
            }

            Label {
                text: Qt.application.version

                wrapMode: Text.Wrap
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                horizontalAlignment: Text.AlignHCenter

                color: Theme.highlightColor
                textFormat: Text.PlainText
            }

            Label {
                text: qsTr("Alternative KDE Connect client for Sailfish OS")

                wrapMode: Text.Wrap
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                horizontalAlignment: Text.AlignHCenter

                color: Theme.highlightColor
            }

            SectionHeader {
                text: qsTr("Links")
            }

            Column {
                x: Theme.horizontalPageMargin
                width: page.width - 2 * Theme.horizontalPageMargin

                Label {
                    text:
                        "<a href=\"https://github.com/R1tschY/harbour-sailfishconnect/issues\">" +
                        qsTr("Bug reports on Github") + "</a>"
                    color: Theme.highlightColor
                    linkColor: Theme.primaryColor
                    onLinkActivated: Qt.openUrlExternally(link)
                }

                Label {
                    text:
                        "<a href=\"https://github.com/R1tschY/harbour-sailfishconnect\">" +
                        qsTr("Source code on Github") + "</a>"
                    color: Theme.highlightColor
                    linkColor: Theme.primaryColor
                    onLinkActivated: Qt.openUrlExternally(link)
                }
            }

            SectionHeader {
                text: qsTr("Licence")
            }
            Label {
                x: Theme.horizontalPageMargin
                width: page.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.WordWrap
                text: "This program is free software: you can redistribute it and/or modify" +
                    "it under the terms of the GNU General Public License as published by " +
                    "the Free Software Foundation, either version 3 of the License, or " +
                    "(at your option) any later version. <br/><br/>" +

                    "This program is distributed in the hope that it will be useful, " +
                    "but WITHOUT ANY WARRANTY; without even the implied warranty of " +
                    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the " +
                    "GNU General Public License for more details.<br/><br/>" +

                    "You should have received a copy of the GNU General Public License " +
                    "along with this program. If not, see <a href=\"http://www.gnu.org/licenses/\">http://www.gnu.org/licenses/</a>." +
                    "<br/><br/>"
                textFormat: Text.StyledText
                color: Theme.highlightColor
                font.pixelSize: Theme.fontSizeSmall
                linkColor: Theme.primaryColor
                onLinkActivated: Qt.openUrlExternally(link)
            }
        }

        VerticalScrollDecorator { flickable: flickable }
    }
}
