import QtQuick 2.0
import Sailfish.Silica 1.0

BackgroundItem {
    id: root
    width: parent.width
    height: Theme.itemSizeMedium
    property alias source: image.source
    property alias title: _title.text

    Rectangle {
        id: imageBox
        height: Theme.itemSizeMedium
        width: Theme.itemSizeMedium
        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: Theme.rgba(Theme.primaryColor, 0.1)
            }
            GradientStop {
                position: 1.0
                color: Theme.rgba(Theme.primaryColor, 0.05)
            }
        }

        HighlightImage {
            id: image
            anchors.centerIn: parent
            highlighted: root.highlighted
        }
    }

    Label {
        id: _title
        truncationMode: TruncationMode.Fade

        anchors {
            left: imageBox.right
            leftMargin: Theme.paddingLarge
            right: parent.right
            verticalCenter: parent.verticalCenter
        }
    }
}
