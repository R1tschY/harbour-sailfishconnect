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

// #ifndef SAILFISHCONNECT_MPRISPLAYERSMODEL_H
// #define SAILFISHCONNECT_MPRISPLAYERSMODEL_H

// #include <QAbstractListModel>

// class Device;

// namespace SailfishConnect {

// class MprisRemotePlugin;

// class MprisPlayersModel : public QAbstractListModel
// {
//     Q_OBJECT

//     Q_PROPERTY(QString deviceId READ deviceId WRITE setDeviceId)

// public:
//     explicit MprisPlayersModel(QObject *parent = nullptr);

//     enum ExtraRoles {
//         Player = Qt::UserRole,
//         PlayerNameRole,
//         IsPlayingRole,
//         CurrentSongRole,
//         TitleRole,
//         ArtistRole,
//         AlbumRole,
//         AlbumArtUrlRole,
//         VolumeRole,
//         LengthRole,
//         PositionRole,
//         PlayAllowedRole,
//         PauseAllowedRole,
//         GoNextAllowedRole,
//         GoPreviousAllowedRole,
//         SeekAllowedRole,
//         SetVolumeAllowedRole,
//     };

//     // Basic functionality:
//     int rowCount(const QModelIndex &parent = QModelIndex()) const override;

//     QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

//     QHash<int, QByteArray> roleNames() const override;

//     QString deviceId() const { return m_deviceId; }
//     void setDeviceId(const QString& value);

// private:
//     QStringList m_players;
//     MprisRemotePlugin* m_plugin = nullptr;
//     QString m_deviceId;
//     Device* m_device = nullptr;

//     void playerAdded(const QString& name);
//     void playerRemoved(const QString& name);
//     void playerUpdated();
//     void pluginDestroyed();
//     void devicePluginsChanged();

//     void setPlugin(MprisRemotePlugin* plugin);

//     void connectPlayer(const QString& name);
// };

// } // namespace SailfishConnect

// #endif // SAILFISHCONNECT_MPRISPLAYERSMODEL_H
