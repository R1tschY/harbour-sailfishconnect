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

#include "devicelistmodel.h"

#include <QIcon>
#include <QLoggingCategory>
#include <QUuid>

#include "../core/daemon.h"
#include "../core/device.h"
#include "../utils/cpphelper.h"

namespace SailfishConnect {

static Q_LOGGING_CATEGORY(logger, "SailfishConnect.DeviceListModel")

static QUrl deviceTypeToIcon(const QString& deviceType)
{
    // TODO: move to qml part
    if (deviceType == QLatin1String("smartphone")
            || deviceType == QLatin1String("phone"))
        return QStringLiteral("image://theme/icon-m-phone");

    if (deviceType == QLatin1String("tablet"))
        return QStringLiteral("image://theme/icon-m-tablet");

    return QStringLiteral("image://theme/icon-m-computer");
}

DeviceListModel::DeviceListModel(QObject *parent)
    : QAbstractListModel(parent), m_uuid(QUuid().toString())
{
    m_daemon = Daemon::instance();

    m_daemon->acquireDiscoveryMode(m_uuid);
    m_devices = m_daemon->devicesList();       

    connect(m_daemon, &Daemon::deviceAdded,
            this, &DeviceListModel::deviceIdAdded);
    connect(m_daemon, &Daemon::deviceRemoved,
            this, &DeviceListModel::deviceIdRemoved);

    for (auto& device : asConst(m_devices)) {
        connectDevice(device);
    }
}

DeviceListModel::~DeviceListModel()
{
    m_daemon->releaseDiscoveryMode(m_uuid);
}

int DeviceListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_devices.length();
}

QVariant DeviceListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_devices.length())
        return QVariant();

    auto& device = m_devices[index.row()];
    switch (role) {
    case NameRole:
        return device->name();
    case IdRole:
        return device->id();
    case IconUrlRole:
        return deviceTypeToIcon(device->id());
    case TrustedRole:
        return device->isTrusted();
    case ReachableRole:
        return device->isReachable();
    case HasPairingRequestsRole:
        return device->hasPairingRequests();
    }

    return QVariant();
}

QHash<int, QByteArray> DeviceListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles.insert(NameRole, "name");
    roles.insert(IdRole, "id");
    roles.insert(IconUrlRole, "iconUrl");
    roles.insert(TrustedRole, "trusted");
    roles.insert(ReachableRole, "reachable");
    roles.insert(HasPairingRequestsRole, "hasPairingRequests");
    return roles;
}

void DeviceListModel::deviceIdAdded(const QString& id)
{
    auto* device = m_daemon->getDevice(id);
    if (device == nullptr) {
        qCDebug(logger) << "no device with id" << id;
        return;
    }
    if (m_devices.contains(device)) {
        qCDebug(logger) << "we have already device with id" << id;
        return;
    }

    int row = indexOfDevice(id);
    if (row < 0) {
        beginInsertRows(QModelIndex(), m_devices.length(), m_devices.length());
        m_devices.append(device);
        endInsertRows();
    }

    connectDevice(device);
}

void DeviceListModel::deviceIdRemoved(const QString& id)
{
    int row = indexOfDevice(id);
    if (row < 0) {
        qCDebug(logger) << "no device with id" << id;
        return;
    }

    deviceRemoved(m_devices[row]);
}

void DeviceListModel::deviceRemoved(Device* device)
{
    int index = m_devices.indexOf(device);
    if (index < 0)
        return;

    disconnectDevice(device);

    beginRemoveRows(QModelIndex(), index, index);
    m_devices.removeAt(index);
    endRemoveRows();
}

void DeviceListModel::deviceDataChanged(
        Device *device, const QVector<int> &roles)
{
    int row = m_devices.indexOf(device);
    if (row < 0)
        return;

    dataChanged(index(row), index(row), roles);
}

void DeviceListModel::connectDevice(Device *device)
{
    connect(device, &Device::nameChanged, this, [=]{
        deviceDataChanged(device, {Qt::DisplayRole, NameRole});
    });
    connect(device, &Device::trustedChanged, this, [=]{
        deviceDataChanged(device, {TrustedRole});
    });
    connect(device, &Device::reachableChanged, this, [=]{
        deviceDataChanged(device, {ReachableRole});
    });
    connect(device, &Device::hasPairingRequestsChanged, this, [=]{
        deviceDataChanged(device, {HasPairingRequestsRole});
    });
    connect(device, &Device::destroyed, this, [=]{
        qCCritical(logger) << "device destroyed with id" << device->id();
        deviceRemoved(device);
    });
}

void DeviceListModel::disconnectDevice(Device *device)
{
    disconnect(device, nullptr, this, nullptr);
}

int DeviceListModel::indexOfDevice(const QString &id)
{
    auto findIter = std::find_if(m_devices.cbegin(), m_devices.cend(),
                                 [&](Device* device){
        return device->id() == id;
    });
    if (findIter == m_devices.cend()) {
        return -1;
    } else {
        return int(std::distance(m_devices.cbegin(), findIter));
    }
}

} // namespace SailfishConnect
