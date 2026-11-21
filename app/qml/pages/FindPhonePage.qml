/*
 * Copyright 2026 Manuel FLURY <zx97@users.noreply.github.com>.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    objectName: "findPhonePage"
    property string deviceId

    allowedOrientations: defaultAllowedOrientations

    Column {
        anchors.centerIn: parent
        width: parent.width - 2 * Theme.horizontalPageMargin
        spacing: Theme.paddingLarge

        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Find my phone")
            font.pixelSize: Theme.fontSizeHuge
            color: Theme.highlightColor
        }

        Label {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            text: qsTr("The phone is ringing. Tap the button below to stop it.")
            color: Theme.primaryColor
        }

        Button {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Stop ringing")
            onClicked: {
                ui.stopFindPhone(page.deviceId)
                pageStack.pop()
            }
        }
    }
}
