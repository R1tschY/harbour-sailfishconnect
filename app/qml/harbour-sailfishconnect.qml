/*
 * Copyright 2018 Richard Liebscher <richard.liebscher@gmail.com>.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import Sailfish.Silica 1.0
import Sailfish.Share 1.0
import Nemo.DBus 2.0
import "pages"

ApplicationWindow
{
    id: window

    initialPage: Component { DeviceListPage { } }
    cover: Qt.resolvedUrl("cover/CoverPage.qml")
    allowedOrientations: defaultAllowedOrientations

    // Must match <OrganizationName>.<ApplicationName> of the desktop file.
    readonly property string shareService:
        "harbour-sailfishconnect.harbour-sailfishconnect"

    // Parses the share() D-Bus argument into a list of resources.
    ShareAction {
        id: shareActionParser
    }

    // Share dialog receiver. The method name "sendto" and this object path
    // must match X-Share-Methods of the desktop file.
    DBusAdaptor {
        id: shareTarget

        service: window.shareService
        path: "/share/sendto"
        iface: "org.sailfishos.share"

        function share(shareConfiguration) {
            shareActionParser.loadConfiguration(shareConfiguration);
            var resources = shareActionParser.resources;
            var filePaths = localResourcePaths(resources);
            if (filePaths.length > 0) {
                window.openSharePage({ filePaths: filePaths, text: "" });
                return;
            }
            var text = textResource(resources);
            if (text === "") {
                console.warn("Share request contains no supported resource");
                return;
            }
            window.openSharePage({ filePaths: [], text: text });
        }
    }

    function localResourcePaths(resources) {
        var paths = [];
        if (!resources)
            return paths;

        // Files only: bare path strings, maps with a filePath key, or
        // file:// URIs. Text or link resources are not supported (yet).
        for (var i = 0; i < resources.length; ++i) {
            var resource = resources[i];
            if (typeof resource !== "string")
                resource = resource && resource.filePath
                           ? resource.filePath.toString() : "";
            if (resource.indexOf("file://") === 0)
                resource = resource.substring(7);
            if (resource.indexOf("/") === 0)
                paths.push(resource);
        }

        return paths;
    }

    function textResource(resources) {
        var texts = [];
        if (!resources)
            return "";

        // Text shares arrive as maps with a data or status key, or as bare
        // strings that are not paths or URIs.
        for (var i = 0; i < resources.length; ++i) {
            var resource = resources[i];
            if (typeof resource === "string") {
                if (resource.indexOf("/") !== 0
                        && resource.indexOf("://") < 0)
                    texts.push(resource);
                continue;
            }
            if (!resource)
                continue;
            var content = resource.status ? resource.status.toString()
                      : resource.data ? resource.data.toString() : "";
            if (content !== "")
                texts.push(content);
        }

        return texts.join("\n");
    }

    function openSharePage(payload) {
        if (typeof pageStack === "undefined")
            return; // happens when using QmlLive

        // Works also when only the daemon part of the app is running.
        window.activate();
        ui.showMainWindow();

        pageStack.push(Qt.resolvedUrl("pages/ShareSendPage.qml"), payload);
    }
}
