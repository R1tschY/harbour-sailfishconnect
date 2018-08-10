import QtQuick 2.0
import Sailfish.Silica 1.0
import SailfishConnect.UI 0.1
import SailfishConnect.Core 0.1
import SailfishConnect.Mpris 0.1

SilicaListView {
    property Device _device: daemon.getDevice(deviceId)

    id: mprisView
    width: parent.width
    height: childrenRect.height
    visible:
        _device.isReachable && _device.isTrusted &&
        _device.supportedPlugins.indexOf(
            "SailfishConnect::MprisRemotePlugin") >= 0

    header: SectionHeader {
        text: "MPRIS"
        id: header
    }

    delegate: mprisPlayerDelegate
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
        width: parent.width - 2 * x
        color: Theme.rgba(Theme.highlightColor, 0.6)
        opacity: enabled ? 1.0 : 0

        Behavior on opacity { FadeAnimation { duration: 300 } }
    }


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
                    visible: player.hasPosition
                    width: parent.width
                    minimumValue: 0
                    maximumValue: player.length
                    value: 0
                    enabled: player.seekAllowed

                    Component.onCompleted: {
                        player.propertiesChange.connect(updatePosition)

                        updatePosition()
                    }

                    function updatePosition() {
                        positionSlider.value = player.position
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
}
