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

#ifndef DEVICELISTMODEL_H
#define DEVICELISTMODEL_H

#include <QAbstractListModel>
#include <QList>

class Daemon;
class Device;

namespace SailfishConnect {

class DeviceListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum ExtraRoles {
        NameRole = Qt::UserRole,
        IdRole,
        IconUrlRole,
        TrustedRole,
        ReachableRole,
        HasPairingRequestsRole,
        WaitsForPairingRole
    };

    explicit DeviceListModel(QObject *parent = 0);
    ~DeviceListModel();

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QHash<int, QByteArray> roleNames() const override;

private:
    Daemon* m_daemon;
    QList<Device*> m_devices;
    QString m_id;

    void deviceIdAdded(const QString& id);
    void deviceIdRemoved(const QString& id);
    void deviceRemoved(Device *device);
    void deviceDataChanged(Device* device, const QVector<int> &roles);

    void connectDevice(Device* device);
    void disconnectDevice(Device* device);

    int indexOfDevice(const QString& id);
    Device* getDevice(const QString& id);
};

} // namespace SailfishConnect

#endif // DEVICELISTMODEL_H
