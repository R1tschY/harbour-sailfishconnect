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
import SailfishConnect.UI 0.2
import SailfishConnect.Core 0.2
import SailfishConnect.Mpris 0.2

SilicaListView {
    id: mprisView
    width: parent.width
    height: Math.max(contentItem.childrenRect.height, Theme.itemSizeExtraLarge)
    visible:
        _device.isReachable && _device.isTrusted
        && _device.loadedPlugins.indexOf(
            "SailfishConnect::MprisRemotePlugin") >= 0

    header: SectionHeader {
        text: qsTr("Multimedia Controls")
        id: header
    }

    delegate: mprisPlayerDelegate
    model: MprisPlayersModel {
        deviceId: page.deviceId
    }

    Label {
        id: replacement

        anchors.bottom: mprisView.bottom
        x: Theme.horizontalPageMargin
        width: parent.width - 2 * x

        enabled: mprisView.count === 0
        opacity: enabled ? 1.0 : 0

        text: qsTr("No remote players")
        wrapMode: Text.Wrap
        horizontalAlignment: Text.AlignHCenter
        font {
            pixelSize: Theme.fontSizeExtraLarge
            family: Theme.fontFamilyHeading
        }
        color: Theme.rgba(Theme.highlightColor, 0.6)

        Behavior on opacity { FadeAnimation { duration: 300 } }
    }


    Component {
        id: mprisPlayerDelegate

        Column {
            id: listItem
            width: parent.width
            spacing: Theme.paddingSmall

            Label {
                text: playerName
                color: Theme.highlightColor
                width: listItem.width
                truncationMode: TruncationMode.Fade
                textFormat: Text.PlainText
            }

            Label {
                text: song
                color: Theme.highlightColor
                width: listItem.width
                truncationMode: TruncationMode.Fade
                textFormat: Text.PlainText
                anchors.horizontalCenter: parent.horizontalCenter
                font.weight: Font.Bold
            }

            Item {
                width: parent.width
                height: Theme.itemSizeMedium

                IconButton {
                    id: playBtn
                    icon.source: (isPlaying && (pauseAllowed || playAllowed)) ?
                        "image://theme/icon-m-pause"
                        : "image://theme/icon-m-play"
                    anchors {
                        horizontalCenter: parent.horizontalCenter
                        verticalCenter: parent.verticalCenter
                    }
                    enabled: pauseAllowed || playAllowed
                    onClicked: player.playPause()
                }

                IconButton {
                    icon.source: "image://theme/icon-m-previous"
                    anchors {
                        top: playBtn.top
                        right: playBtn.left
                    }
                    visible: goPreviousAllowed
                    onClicked: player.previous()
                }

                IconButton {
                    icon.source: "image://theme/icon-m-next"
                    anchors {
                        top: playBtn.top
                        left: playBtn.right
                    }
                    visible: goNextAllowed
                    onClicked: player.next()
                }
            }

            Slider {
                id: positionSlider
                visible: player.hasPosition && player.length > 0
                width: parent.width
                minimumValue: 0
                maximumValue: Math.max(player.length, 1)
                value: 0
                enabled: player.seekAllowed
                onReleased: {
                    if (player) {
                        player.position = positionSlider.value
                    }
                }

                Component.onCompleted: {
                    player.propertiesChanged.connect(updatePosition)

                    updatePosition()
                }

                function updatePosition() {
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
                onTriggered: positionSlider.updatePosition()
            }
        }
    }
}
