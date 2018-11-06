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

#include "pluginloader.h"

#include <QPluginLoader>
#include <QtPlugin>
#include <QJsonArray>
#include <QJsonObject>
#include <QLoggingCategory>

#include "corelogging.h"
#include "device.h"
#include "kdeconnectplugin.h"
#include <sailfishconnect/helper/cpphelper.h>

using namespace SailfishConnect;


class PluginLoader {
public:
    virtual ~PluginLoader() = default;

    virtual QJsonObject metadata() = 0;
    virtual QObject* instance() = 0;
    virtual QString errorString() = 0;
};

namespace {

class StaticPluginLoader : public PluginLoader {
public:
    StaticPluginLoader(const QStaticPlugin& staticPlugin)
    : staticPlugin(staticPlugin)
    { }

    QJsonObject metadata() override
    {
        return staticPlugin.metaData();
    }

    QObject* instance() override
    {
        return staticPlugin.instance();
    }

    QString errorString() override
    {
        return QString();
    }

private:
    QStaticPlugin staticPlugin;
};

class DynamicPluginLoader : public PluginLoader {
public:
    DynamicPluginLoader(const QString &fileName)
    : pluginLoader(fileName)
    { }

    QJsonObject metadata() override
    {
        return pluginLoader.metaData();
    }

    QObject* instance() override
    {
        return pluginLoader.instance();
    }

    QString errorString() override
    {
        return pluginLoader.errorString();
    }

private:
    QPluginLoader pluginLoader;
};

//

QStringList toStringList(const QJsonValue& value)
{
    const auto array = value.toArray();

    QStringList list;
    list.reserve(array.size());
    for (auto entry : array) {
        auto strEntry = entry.toString();
        if (!strEntry.isNull()) {
            list.append(entry.toString());
        }
    }
    return list;
}

} // namespace

PluginManager::PluginListEntry PluginManager::createPluginEntry(
        std::unique_ptr<PluginLoader> pluginLoader)
{
    QJsonObject metadata = pluginLoader->metadata();
    if (metadata.value(QStringLiteral("IID")).toString()
            != QStringLiteral("SailfishConnect.Plugin"))
    {
        return PluginListEntry();
    }

    auto pluginMetadata = metadata.value(QStringLiteral("MetaData")).toObject();
    auto pluginId = pluginMetadata.value("Id").toString();
    if (pluginId.isEmpty()) {
        qCDebug(coreLogger) << "Could not load plugin with empty id with "
                               "metadata:" << pluginMetadata;
        return PluginListEntry();
    }

    auto* factory = qobject_cast<SailfishConnectPluginFactory*>(
                pluginLoader->instance());
    if (!factory) {
        qCDebug(coreLogger).nospace()
                << "Could not load plugin: " << pluginId
                << " (" << pluginLoader->errorString() << ")";
        return PluginListEntry();
    }

    return PluginListEntry {
            pluginId,
            toStringList(pluginMetadata.value(
                             QStringLiteral("IncomingCapabilities"))).toSet(),
            toStringList(pluginMetadata.value(
                             QStringLiteral("OutcomingCapabilities"))).toSet(),
            pluginMetadata.value(
                             QStringLiteral("EnabledByDefault")).toBool(true),
            factory,
            std::move(pluginLoader)
    };
}

PluginManager* PluginManager::instance()
{
    static PluginManager instance;
    return &instance;
}

PluginManager::PluginManager()
{
    const auto staticPlugins = QPluginLoader::staticPlugins();
    for (const QStaticPlugin& staticPlugin : staticPlugins) {
        auto entry = createPluginEntry(
                    makeUniquePtr<StaticPluginLoader>(staticPlugin));
        if (!entry.id.isEmpty()) {
            plugins[entry.id] = std::move(entry);
        }
    }

    for (auto& keyValue : asConst(plugins)) {
       keyValue.second.factory->registerTypes();
    }

    qCDebug(coreLogger) << "loaded plugins:" << getPluginList();
}

QStringList PluginManager::getPluginList() const
{
    QStringList list;
    list.reserve(plugins.size());
    for (auto& keyValue : asConst(plugins)) {
       list.append(keyValue.first);
    }
    return list;
}

KdeConnectPlugin* PluginManager::instantiatePluginForDevice(const QString& pluginId, Device* device) const
{
    auto entryIter = plugins.find(pluginId);
    if (entryIter == plugins.end()) {
        qCDebug(coreLogger) << "No plugin with id" << pluginId;
        return nullptr;
    }

    auto& entry = entryIter->second;
    KdeConnectPlugin* plugin = entry.factory->create(
                device, entry.id, entry.outgoingCapabilities);
    if (!plugin) {
        qCDebug(coreLogger) << "Error loading plugin";
        return nullptr;
    }

    qCDebug(coreLogger) << "Loaded plugin:" << entry.id;
    return plugin;
}

QStringList PluginManager::incomingCapabilities(const QString &pluginId) const
{
    auto entryIter = plugins.find(pluginId);
    if (entryIter == plugins.end()) {
        return QStringList();
    }
    return entryIter->second.incomingCapabilities.toList();
}

QStringList PluginManager::outgoingCapabilities(const QString &pluginId) const
{
    auto entryIter = plugins.find(pluginId);
    if (entryIter == plugins.end()) {
        return QStringList();
    }
    return entryIter->second.outgoingCapabilities.toList();
}

bool PluginManager::enabledByDefault(const QString &pluginId) const
{
    auto entryIter = plugins.find(pluginId);
    if (entryIter == plugins.end()) {
        return true;
    }
    return entryIter->second.enabledByDefault;
}

QString PluginManager::pluginName(const QString &pluginId) const
{
    auto entryIter = plugins.find(pluginId);
    if (entryIter == plugins.end()) {
        return QString();
    }
    return entryIter->second.factory->name();
}

QString PluginManager::pluginDescription(const QString &pluginId) const
{
    auto entryIter = plugins.find(pluginId);
    if (entryIter == plugins.end()) {
        return QString();
    }
    return entryIter->second.factory->description();
}

QString PluginManager::pluginIconUrl(const QString &pluginId) const
{
    auto entryIter = plugins.find(pluginId);
    if (entryIter == plugins.end()) {
        return QString();
    }
    return entryIter->second.factory->iconUrl();
}

QStringList PluginManager::incomingCapabilities() const
{
    QSet<QString> ret;
    for (auto& keyValue : asConst(plugins)) {
        ret += keyValue.second.incomingCapabilities;
    }
    return ret.toList();
}

QStringList PluginManager::outgoingCapabilities() const
{
    QSet<QString> ret;
    for (auto& keyValue : asConst(plugins)) {
        ret += keyValue.second.outgoingCapabilities;
    }
    return ret.toList();
}

QSet<QString> PluginManager::pluginsForCapabilities(const QSet<QString>& incoming, const QSet<QString>& outgoing)
{
    QSet<QString> pluginIds;

    for (auto& keyValue : asConst(plugins)) {
        auto& entry = keyValue.second;
        bool capabilitiesEmpty =
                entry.incomingCapabilities.isEmpty()
                && entry.outgoingCapabilities.isEmpty();
        bool capabilitiesIntersect =
                outgoing.intersects(entry.incomingCapabilities)
                || incoming.intersects(entry.outgoingCapabilities);

        if (capabilitiesIntersect || capabilitiesEmpty) {
            pluginIds += entry.id;
        } else {
            qCDebug(coreLogger)
                    << "Not loading plugin" << entry.id
                    <<  "because device doesn't support it";
        }
    }

    return pluginIds;
}
