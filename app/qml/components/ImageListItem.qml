import QtQuick 2.0
import Sailfish.Silica 1.0

BackgroundItem {
    id: root
    width: parent.width
    contentHeight: Theme.itemSizeSmall
    property alias icon: image
    property alias text: label.text

    HighlightImage {
        id: image
        anchors.left: parent.left
        anchors.leftMargin: Theme.horizontalPageMargin
        y: (Theme.itemSizeSmall - height)/2
        highlighted: root.highlighted
        opacity: parent.enabled ? 1.0 : 0.4
    }

    Label {
        id: label
        anchors {
            left: image.right
            leftMargin: Theme.paddingMedium
            right: parent.right
        }

        truncationMode: TruncationMode.Fade
    }
}
