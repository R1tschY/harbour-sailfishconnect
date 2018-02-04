import QtQuick 2.0
import Sailfish.Silica 1.0

Dialog {
    property string deviceId

    acceptDestinationAction: PageStackAction.Replace
    acceptDestination: Qt.resolvedUrl("DevicePage.qml")
    acceptDestinationProperties: { deviceId: deviceId }

    DialogHeader {
        id: header
        title: "Confirm pairing"
    }
    Label {
        text: "Pair with " + daemon.getDevice(deviceId).name + "?"
        anchors.top: header.bottom
        x: Theme.horizontalPageMargin
        color: Theme.highlightColor
    }
}
