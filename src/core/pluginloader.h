/**
 * Copyright 2013 Albert Vaca <albertvaka@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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

#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

#include <map>
#include <memory>
#include <QObject>
#include <QSet>
#include <QString>

#include <QJsonObject>

class Device;
class KdeConnectPlugin;
class PluginLoader;

class PluginManager
{

public:
    static PluginManager* instance();

    QStringList getPluginList() const;
    KdeConnectPlugin* instantiatePluginForDevice(const QString& name, Device* device) const;

    QStringList incomingCapabilities(const QString& pluginId) const;
    QStringList outgoingCapabilities(const QString& pluginId) const;

    QStringList incomingCapabilities() const;
    QStringList outgoingCapabilities() const;
    QSet<QString> pluginsForCapabilities(const QSet<QString>& incoming, const QSet<QString>& outgoing);

private:
    PluginManager();

    struct PluginListEntry {
        QString id;
        QSet<QString> incomingCapabilities;
        QSet<QString> outgoingCapabilities;
        std::unique_ptr<PluginLoader> loader;
    };

    std::map<QString, PluginListEntry> plugins;

    PluginListEntry createPluginEntry(
            std::unique_ptr<PluginLoader> pluginLoader);
};

#endif
