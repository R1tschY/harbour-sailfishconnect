import QtQuick 2.0
import Sailfish.Silica 1.0
import SailfishConnect.UI 0.1
import SailfishConnect.Core 0.1
import SailfishConnect.Mpris 0.1

Item {
    Component {
        id: mprisPlayerDelegate

        ListItem {
            id: listItem
            width: parent.width
            height: childrenRect.height

            Column {
                width: parent.width
                id: mprisItems

                Label {
                    text: playerName
                    truncationMode: TruncationMode.Fade
                    textFormat: Text.PlainText
                }

                Label {
                    text: song
                    truncationMode: TruncationMode.Fade // TODO: does not work
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
                    visible: player.position > 0
                    width: parent.width
                    minimumValue: 0
                    maximumValue: length
                    value: player.position // TODO: update on song change
                    enabled: false // TODO: seekAllowed
                }

                Timer {
                    interval: 1000
                    repeat: true
                    running: isPlaying
                    triggeredOnStart: true
                    onTriggered: positionSlider.value = player.position
                }
            }
        }
    }

    width: parent.width
    height: mprisView.height + replacement.height

    SilicaListView {

        id: mprisView
        width: parent.width
        height: childrenRect.height
        // TODO: visible: "mpris" in loadedPlugins

        header: SectionHeader {
            text: "MPRIS"
            id: header
        }

        model: MprisPlayersModel {
            deviceId: page.deviceId
        }

        Label {
            id: replacement
            text: qsTr("No remote player")
            enabled: mprisView.count === 0
            anchors.bottom: mprisView.bottom
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            font {
                pixelSize: Theme.fontSizeExtraLarge
                family: Theme.fontFamilyHeading
            }
            x: Theme.horizontalPageMargin
            width: parent.width - 2*x
            color: Theme.rgba(Theme.highlightColor, 0.6)
            opacity: enabled ? 1.0 : 0

            Behavior on opacity { FadeAnimation { duration: 300 } }
        }

        delegate: mprisPlayerDelegate
    }
}
