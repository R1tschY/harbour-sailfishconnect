import QtQuick 2.0

import QtQuick 2.0
import Sailfish.Silica 1.0
import SailfishConnect.Core 0.2


Page {
    id: page

    SilicaFlickable {
        id: pageFlickable
        anchors.fill: parent
        contentHeight: content.height

        Column {
            id: content

            width: page.width
            spacing: Theme.paddingLarge

            PageHeader {
                title: qsTr("Settings")
            }

            TextSwitch {
                id: runInBackground
                text: qsTr("Run in background")
                description:
                    qsTr("Application runs in background after " +
                         "window is closed.")
                checked: ui.runInBackground
                onClicked: ui.runInBackground = !runInBackground.checked
                automaticCheck: false
            }
        }

        VerticalScrollDecorator { flickable: pageFlickable }
    }
}
