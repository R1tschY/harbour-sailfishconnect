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
import SailfishConnect.UI 0.6
import SailfishConnect.Api 0.6

Column {
    property QtObject device

    width: parent.width
    visible: device && device.isPluginLoaded("sailfishconnect_mprisremote")


    SilicaListView {
        id: mprisView
        width: parent.width
        height: contentHeight

        header: SectionHeader {
            text: i18n("Multimedia Controls")
            id: header
        }

        delegate: mprisPlayerDelegate
        model: MprisPlayersModel {
            deviceId: page.deviceId
        }

        Component {
            id: mprisPlayerDelegate

            BackgroundItem {
                id: listItem
                width: parent.width
                //height: parent.width
                height: hasPosition
                        ? (Theme.itemSizeMedium * 2.5)
                        : (Theme.itemSizeMedium * 2)

                property bool hasPosition: player.hasPosition && player.length > 0

                Label {
                    id: playerNameLabel
                    text: playerName
                    color: Theme.highlightColor
                    truncationMode: TruncationMode.Fade
                    textFormat: Text.PlainText
                    anchors {
                        left: parent.left
                        right: parent.right
                        leftMargin: Theme.horizontalPageMargin
                    }
                }

                Image {
                    id: albumArt
                    source: albumArtUrl
                    width: parent.width
                    asynchronous: true
                    sourceSize.height: parent.width
                    visible: status === Image.Ready
                    fillMode: Image.PreserveAspectCrop
                    opacity: 0.25

                    anchors.fill: parent
                }

                Item {
                    anchors {
                        top: playerNameLabel.bottom
                        left: parent.left
                        leftMargin: Theme.horizontalPageMargin
                        right: parent.right
                    }

                    Label {
                        id: songLabel
                        text: song ? song : ("<" + i18n("Nothing playing") + ">")
                        color: Theme.highlightColor
                        truncationMode: TruncationMode.Fade
                        textFormat: Text.PlainText
                        font.weight: Font.Bold
                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                    }

                    Item {
                        id: playBtns
                        height: Theme.itemSizeSmall
                        anchors {
                            left: parent.left
                            right: parent.right
                            top: songLabel.bottom
                        }

                        IconButton {
                            id: playBtn
                            icon.source: (isPlaying && (pauseAllowed || playAllowed)) ?
                                "image://theme/icon-m-pause"
                                : "image://theme/icon-m-play"
                            anchors.horizontalCenter: parent.horizontalCenter
                            enabled: pauseAllowed || playAllowed
                            onClicked: player.playPause()
                        }

                        IconButton {
                            icon.source: "image://theme/icon-m-previous"
                            anchors.right: playBtn.left
                            visible: goPreviousAllowed
                            onClicked: player.previous()
                        }

                        IconButton {
                            icon.source: "image://theme/icon-m-next"
                            anchors.left: playBtn.right
                            visible: goNextAllowed
                            onClicked: player.next()
                        }
                    }

                    Slider {
                        id: positionSlider

                        anchors {
                            left: parent.left
                            right: parent.right
                            top: playBtns.bottom
                        }
                        visible: hasPosition
                        minimumValue: 0
                        maximumValue: Math.max(player.length, 1)
                        value: 0
                        enabled: player.seekAllowed
                        onReleased: {
                            if (player) {
                                player.position = positionSlider.value
                            }
                        }

                        Connections {
                            target: player
                            onPropertiesChanged: positionSlider.value = player.position
                        }

                        Component.onCompleted: {
                            if (player) {
                                positionSlider.value = player.position
                            }
                        }
                    }

                    Timer {
                        interval: 1000
                        repeat: true
                        running: isPlaying && player.hasPosition
                        triggeredOnStart: false
                        onTriggered: {
                            if (player) {
                                positionSlider.value = player.position
                            }
                        }
                    }
                }
            }
        }
    }


    Label {
        id: replacement

        x: Theme.horizontalPageMargin
        width: parent.width - 2 * x

        visible: mprisView.count === 0
        opacity: visible ? 1.0 : 0

        text: i18n("No remote players")
        wrapMode: Text.Wrap
        horizontalAlignment: Text.AlignHCenter
        font {
            pixelSize: Theme.fontSizeExtraLarge
            family: Theme.fontFamilyHeading
        }
        color: Theme.rgba(Theme.highlightColor, 0.6)

        Behavior on opacity { FadeAnimation { duration: 300 } }
    }
}
