/*
 * Copyright 2020 Richard Liebscher <richard.liebscher@gmail.com>.
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

import QtQuick 2.6
import Sailfish.Silica 1.0
import SailfishConnect.Api 0.7
import "../../components"

Page {
    id: page

    allowedOrientations: Orientation.All

    property QtObject device
    property QtObject config: device.getPluginConfig("kdeconnect_runcommands")

    /// https://stackoverflow.com/a/2117523/1188453
    function uuidv4() {
        return 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) {
            var r = Math.random() * 16 | 0, v = c == 'x' ? r : (r & 0x3 | 0x8);
            return v.toString(16);
        });
    }

    ListModel {
        id: commandsModel

        function reload() {
            clear()

            var commands = JSON.parse(config.get("commands"))
            for (var key in commands) {
                if (!commands.hasOwnProperty(key))
                    continue;

                append({
                    "key": key,
                    "name": commands[key]["name"],
                    "command": commands[key]["command"],
                })
            }
        }

        function addCommand(name, command) {
            append({
                "key": uuidv4(),
                "name": name,
                "command": command,
            })
            save()
        }

        function editCommand(key, name, command) {
            var index = findByKey(key)
            setProperty(index, "name", name)
            setProperty(index, "command", command)
            save()
        }

        function findByKey(key) {
            for (var i = 0; i < count; i++) {
                if (get(i)["key"] == key) {
                    return i
                }
            }
            return -1
        }

        function removeByKey(key) {
            remove(findByKey(key))
            save()
        }

        function save() {
            var result = {}
            for (var i = 0; i < count; i++) {
                var command = get(i)
                result[command["key"]] = {
                    "name": command["name"],
                    "command": command["command"],
                }
            }
            config.set("commands", JSON.stringify(result))
        }

        Component.onCompleted: reload()
    }

    Connections {
        target: config
        onConfigChanged: console.log("CHANGE!!!", config.get("commands"))
    }

    SilicaListView {
        id: view
        anchors.fill: parent

        header: PageHeader {
            title: i18n("Edit commands")
        }

        model: commandsModel

        delegate: ListItem {
            id: listItem
            menu: contextMenu
            width: page.width
            contentHeight: item.height
            ListView.onRemove: animateRemoval(listItem)

            LabelWithDescription {
                id: item
                width: page.width - 2 * Theme.horizontalPageMargin
                x: Theme.horizontalPageMargin
                
                title: name
                description: command

                highlighted: listItem.highlighted
            }

            Component {
                id: contextMenu
                ContextMenu {
                    MenuItem {
                        text: i18n("Edit")
                        onClicked: {
                            var dialog = pageStack.push(
                                Qt.resolvedUrl("AddCommandDialog.qml"),
                                {"key": key, "name": name, "command": command})
                            dialog.accepted.connect(function() {
                                commandsModel.editCommand(
                                    dialog.key, dialog.name.trim(), dialog.command.trim())
                            })
                        }
                    }

                    MenuItem {
                        text: i18n("Remove")
                        onClicked: commandsModel.removeByKey(key)
                    }
                }
            }
        }

        PullDownMenu {
            MenuItem {
                text: i18n("Add new command")
                onClicked: {
                    var dialog = pageStack.push(Qt.resolvedUrl("AddCommandDialog.qml"))
                    dialog.accepted.connect(function() {
                        commandsModel.addCommand(dialog.name.trim(), dialog.command.trim())
                    })
                }
            }
        }

        ViewPlaceholder {
            id: placeholder
            enabled: view.count === 0
            flickable: view
            text: i18n("No commands defined")
        }

        VerticalScrollDecorator { flickable: view }
    }
}
