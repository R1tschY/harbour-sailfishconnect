import QtQuick 2.0
import Sailfish.Silica 1.0

Item {
    property int size: Theme.itemSizeLarge
    property alias source: albumArtImage.source
    property string fallbackIcon: "image://theme/icon-m-media-albums"

    width: size
    height: size

    Rectangle {
        anchors.fill: parent
        color: Theme.rgba(Theme.primaryColor, 0.1)
        visible: !albumArtImage.visible

        Image {
            anchors.centerIn: parent
            source: fallbackIcon
        }
    }

    Image {
        id: albumArtImage

        anchors.fill: parent
        asynchronous: true
        sourceSize.height: size
        visible: status === Image.Ready
        fillMode: Image.PreserveAspectCrop
    }
}
