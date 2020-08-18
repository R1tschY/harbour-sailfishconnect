/*
 * Copyright 2019 Richard Liebscher <richard.liebscher@gmail.com>.
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

#include "devicepluginsmodel.h"

#include <QLoggingCategory>
#include <QPluginLoader>

#include "../dbus/kdeconnect.h"

namespace SailfishConnect {

static Q_LOGGING_CATEGORY(logger, "SailfishConnect::DevicePluginsModel");


DevicePluginsModel::DevicePluginsModel(QObject* parent)
    : QAbstractListModel(parent)
{
    const QVector<QStaticPlugin> staticPlugins = QPluginLoader::staticPlugins();
    for (auto& staticPlugin : staticPlugins) {
        QJsonObject jsonMetadata = staticPlugin.metaData().value(QStringLiteral("MetaData")).toObject();
        KPluginMetaData metadata(jsonMetadata, QString());
        if (metadata.serviceTypes().contains(QStringLiteral("KdeConnect/Plugin"))) {
            m_plugins.append(metadata);
        }
    }
}

int DevicePluginsModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;

    return m_plugins.size();
}

QVariant DevicePluginsModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= m_plugins.length())
        return QVariant();

    auto& metadata = m_plugins[index.row()];
    switch (role) {
    case IdRole:
        return metadata.pluginId();
    case NameRole:
        return metadata.name();
    case DescriptionRole:
        return metadata.description();
    case EnabledRole:
        // TODO: request enabled plugins only once
        return device_ ? device_->isPluginEnabled(metadata.pluginId()) : false;
    case IconUrlRole:
        return metadata.iconName();
    }

    return QVariant();
}

bool DevicePluginsModel::setData(
    const QModelIndex& idx, const QVariant& value, int role)
{
    if (!idx.isValid() || idx.row() >= m_plugins.length() || device_ == nullptr)
        return false;

    if (role == EnabledRole) {
        // setPluginEnabled triggers pluginsChanged triggers setDevice
        // which resets the model
        if (data(idx, EnabledRole) != value) {
            auto pluginId = m_plugins[idx.row()].pluginId();
            device_->setPluginEnabled(pluginId, value.toBool());
        }
        return true;
    }

    return false;
}

Qt::ItemFlags DevicePluginsModel::flags(const QModelIndex& index) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
}

QHash<int, QByteArray> DevicePluginsModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "pluginId";
    roles[NameRole] = "pluginName";
    roles[EnabledRole] = "pluginEnabled";
    roles[IconUrlRole] = "pluginIconUrl";
    roles[DescriptionRole] = "pluginDescription";
    return roles;
}

DeviceApi* DevicePluginsModel::device()
{
    return device_;
}

void DevicePluginsModel::setDevice(DeviceApi* value)
{
    beginResetModel();

    if (device_) {
        device_->disconnect(this);
    }

    device_ = value;

    if (device_) {
        connect(device_, &DeviceApi::destroyed,
            this, [&]() { setDevice(nullptr); });
    }

    endResetModel();

    emit deviceChanged();
}

} // namespace SailfishConnect
