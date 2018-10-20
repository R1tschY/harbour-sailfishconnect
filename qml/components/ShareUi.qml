import QtQuick 2.0
import Sailfish.Silica 1.0
import SailfishConnect.Core 0.2
import SailfishConnect.Share 0.2
import Sailfish.Pickers 1.0

Column {
    width: parent.width
    visible:
        _device.isReachable && _device.isTrusted &&
        _device.supportedPlugins.indexOf(
            "SailfishConnect::SharePlugin") >= 0

    property var plugin: _device.plugin("SailfishConnect::SharePlugin")

    SectionHeader { text: qsTr("Share") }

    IconButton {
       icon.source: "image://theme/icon-m-share"
       anchors.horizontalCenter: parent.horizontalCenter
       onClicked: pageStack.push(filePickerPage)
    }

    Component {
        id: filePickerPage
        ContentPickerPage {
            title: "Select file to send"
            onSelectedContentPropertiesChanged: {
                plugin.share(selectedContentProperties.url)
            }
        }
    }
}
