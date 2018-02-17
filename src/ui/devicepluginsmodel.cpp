#include "devicepluginsmodel.h"

#include <QLoggingCategory>

#include "../core/daemon.h"
#include "../core/device.h"
#include "../core/kdeconnectplugin.h"
#include "../core/pluginloader.h"

namespace SailfishConnect {

static Q_LOGGING_CATEGORY(logger, "SailfishConnect::DevicePluginsModel")

DevicePluginsModel::DevicePluginsModel(QObject *parent)
    : QAbstractListModel(parent)
{ }

int DevicePluginsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return pluginIds_.size();
}

QVariant DevicePluginsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= pluginIds_.length())
        return QVariant();

    auto& pluginId = pluginIds_[index.row()];
    switch (role) {
    case NameRole:
        return PluginManager::instance()->pluginName(pluginId);
    case DescriptionRole:
        return PluginManager::instance()->pluginDescription(pluginId);
    case EnabledRole:
        return device_->isPluginEnabled(pluginId);
    case IconUrlRole:
        return PluginManager::instance()->pluginIconUrl(pluginId);
    }

    return QVariant();
}

bool DevicePluginsModel::setData(
        const QModelIndex& idx, const QVariant& value, int role)
{
    if (!idx.isValid() || idx.row() >= pluginIds_.length())
        return false;

    if (role == EnabledRole) {
        // setPluginEnabled triggers pluginsChanged triggers setDevice
        // which resets the model
        if (data(idx, EnabledRole) != value) {
            auto& pluginId = pluginIds_[idx.row()];
            device_->setPluginEnabled(pluginId, value.toBool());
        }
        return true;
    }

    return false;
}

Qt::ItemFlags DevicePluginsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() >= pluginIds_.length())
        return Qt::NoItemFlags;

    return Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled |
            Qt::ItemNeverHasChildren;
}

QHash<int, QByteArray> DevicePluginsModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "pluginName";
    roles[EnabledRole] = "pluginEnabled";
    roles[IconUrlRole] = "pluginIconUrl";
    roles[DescriptionRole] = "pluginDescription";
    return roles;
}

QString DevicePluginsModel::deviceId()
{
    return (device_) ? device_->id() : QString();
}

void DevicePluginsModel::setDeviceId(const QString& value)
{
    setDevice(Daemon::instance()->getDevice(value));
}

void DevicePluginsModel::setDevice(Device* value)
{
    beginResetModel();

    if (device_) {
        device_->disconnect(this);
        pluginIds_.clear();
    }

    device_ = value;

    if (device_) {
        connect(device_, &Device::destroyed,
                this, [&](){ setDevice(nullptr); });

        pluginIds_ = device_->supportedPlugins();
    }

    endResetModel();

    emit deviceIdChanged();
}

} // namespace SailfishConnect
