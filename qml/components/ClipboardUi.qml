import QtQuick 2.0
import Sailfish.Silica 1.0
import SailfishConnect.Core 0.2

Column {
    visible:
        _device.isReachable && _device.isTrusted &&
        _device.supportedPlugins.indexOf(
            "SailfishConnect::ClipboardPlugin") >= 0

    SectionHeader { text: qsTr("Copyboard") }
    Label {
        wrapMode: Text.Wrap
        width: mainColumn.width
        color: Theme.highlightColor
        font.pixelSize: Theme.fontSizeSmall
        text: qsTr("To send the clipboard content this window " +
                   "has to be in the foreground. For receiving " +
                   "a clipboard content this is not required.")
    }
}
