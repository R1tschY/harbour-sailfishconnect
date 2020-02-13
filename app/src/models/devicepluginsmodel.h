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

#pragma once

#include <QAbstractListModel>
#include <QStringList>
#include <QVector>
#include <KPluginMetaData>


namespace SailfishConnect {

class DeviceApi;

class DevicePluginsModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(DeviceApi* device
               READ device
               WRITE setDevice
               NOTIFY deviceChanged)

public:
    enum ExtraRoles {
        NameRole = Qt::DisplayRole,
        EnabledRole = Qt::CheckStateRole,

        DescriptionRole = Qt::UserRole,
        IconUrlRole,
        IdRole,
    };

    explicit DevicePluginsModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index,
                  int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& idx,
                 const QVariant& value,
                 int role) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    DeviceApi* device();
    void setDevice(DeviceApi* value);

    QHash<int, QByteArray> roleNames() const override;

signals:
    void deviceChanged();

private:
    DeviceApi* device_ = nullptr;

    QVector<KPluginMetaData> m_plugins;

    void update();
};

} // namespace SailfishConnect
