// /*
//  * Copyright 2018 Richard Liebscher <richard.liebscher@gmail.com>.
//  *
//  * This program is free software: you can redistribute it and/or modify
//  * it under the terms of the GNU General Public License as published by
//  * the Free Software Foundation, either version 3 of the License, or
//  * (at your option) any later version.
//  *
//  * This program is distributed in the hope that it will be useful,
//  * but WITHOUT ANY WARRANTY; without even the implied warranty of
//  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  * GNU General Public License for more details.
//  *
//  * You should have received a copy of the GNU General Public License
//  * along with this program.  If not, see <http://www.gnu.org/licenses/>.
//  */

// #include "mprisplayersmodel.h"

// #include <algorithm>

// #include <QLoggingCategory>

// #include <daemon.h>
// #include <device.h>
// #include <qtcompat_p.h>
// #include "../plugins/mprisremote/mprisremoteplugin.h"

// namespace SailfishConnect {

// static Q_LOGGING_CATEGORY(logger, "sailfishconnect.mpris-players-model")

// MprisPlayersModel::MprisPlayersModel(QObject *parent)
//     : QAbstractListModel(parent)
// {
// }

// int MprisPlayersModel::rowCount(const QModelIndex &parent) const
// {
//     if (parent.isValid())
//         return 0;

//     return m_players.size();
// }

// QVariant MprisPlayersModel::data(const QModelIndex &index, int role) const
// {
//     if (!index.isValid() || index.row() >= m_players.size())
//         return QVariant();

//     auto player = m_plugin->player(m_players[index.row()]);
//     switch (role) {
//     case Player:
//         return QVariant::fromValue<QObject*>(player);
//     case PlayerNameRole:
//         return m_players[index.row()];
//     case IsPlayingRole:
//         return player->isPlaying();
//     case CurrentSongRole:
//         return player->currentSong();
//     case TitleRole:
//         return player->title();
//     case ArtistRole:
//         return player->artist();
//     case AlbumRole:
//         return player->album();
//     case AlbumArtUrlRole:
//         return player->albumArtUrl();
//     case VolumeRole:
//         return player->volume();
//     case LengthRole:
//         return player->length();
//     case PositionRole:
//         return player->position();
//     case PlayAllowedRole:
//         return player->playAllowed();
//     case PauseAllowedRole:
//         return player->pauseAllowed();
//     case GoNextAllowedRole:
//         return player->goNextAllowed();
//     case GoPreviousAllowedRole:
//         return player->goPreviousAllowed();
//     case SeekAllowedRole:
//         return player->seekAllowed();
//     case SetVolumeAllowedRole:
//         return player->setVolumeAllowed();
//     }

//     return QVariant();
// }

// QHash<int, QByteArray> MprisPlayersModel::roleNames() const
// {
//     QHash<int, QByteArray> roles;
//     roles[Player] = "player";
//     roles[PlayerNameRole] = "playerName";
//     roles[IsPlayingRole] = "isPlaying";
//     roles[CurrentSongRole] = "song";
//     roles[TitleRole] = "title";
//     roles[ArtistRole] = "artist";
//     roles[AlbumRole] = "album";
//     roles[AlbumArtUrlRole] = "albumArtUrl";
//     roles[VolumeRole] = "volume";
//     roles[LengthRole] = "length";
//     roles[PositionRole] = "position";
//     roles[PlayAllowedRole] = "playAllowed";
//     roles[PauseAllowedRole] = "pauseAllowed";
//     roles[GoNextAllowedRole] = "goNextAllowed";
//     roles[GoPreviousAllowedRole] = "goPreviousAllowed";
//     roles[SeekAllowedRole] = "seekAllowed";
//     roles[SetVolumeAllowedRole] = "setVolumeAllowed";
//     return roles;
// }

// void MprisPlayersModel::setDeviceId(const QString& value)
// {
//     if (value == m_deviceId)
//         return;

//     if (m_device) {
//         m_device->disconnect(this);
//     }

//     m_device = Daemon::instance()->getDevice(value);
//     if (!m_device) {
//         m_deviceId = QString();
//         setPlugin(nullptr);
//         return;
//     }

//     m_deviceId = value;

//     connect(
//         m_device, &Device::pluginsChanged,
//         this, &MprisPlayersModel::devicePluginsChanged);

//     devicePluginsChanged();
// }

// void MprisPlayersModel::setPlugin(MprisRemotePlugin* plugin)
// {
//     if (plugin == m_plugin)
//         return;

//     beginResetModel();
//     if (m_plugin) {
//         m_plugin->disconnect(this);
//         for (auto player : asConst(m_players)) {
//             m_plugin->player(player)->disconnect(this);
//         }
//         m_players.clear();
//     }

//     m_plugin = plugin;
//     if (m_plugin) {
//         connect(m_plugin, &MprisRemotePlugin::destroyed,
//                 this, &MprisPlayersModel::pluginDestroyed);
//         connect(m_plugin, &MprisRemotePlugin::playerAdded,
//                 this, &MprisPlayersModel::playerAdded);
//         connect(m_plugin, &MprisRemotePlugin::playerRemoved,
//                 this, &MprisPlayersModel::playerRemoved);

//         m_players = m_plugin->players();
//         for (auto player : asConst(m_players)) {
//             connectPlayer(player);
//         }
//     }

//     endResetModel();
// }

// void MprisPlayersModel::connectPlayer(const QString &name)
// {
//     auto player = m_plugin->player(name);
//     connect(player, &MprisPlayer::propertiesChanged,
//             this, &MprisPlayersModel::playerUpdated);
// }

// void MprisPlayersModel::playerAdded(const QString& name)
// {
//     auto iter = std::lower_bound(m_players.cbegin(), m_players.cend(), name);
//     if (iter != m_players.cend() && *iter == name) {
//         return;
//     }

//     auto insert_pos = std::distance(m_players.cbegin(), iter);

//     beginInsertRows(QModelIndex(), insert_pos, insert_pos);

//     m_players.insert(insert_pos, name);
//     connectPlayer(name);

//     endInsertRows();
// }

// void MprisPlayersModel::playerRemoved(const QString& name)
// {
//     auto remove_pos = m_players.indexOf(name);
//     if (remove_pos < 0) {
//         return;
//     }

//     beginRemoveRows(QModelIndex(), remove_pos, remove_pos);
//     m_players.removeAt(remove_pos);
//     endRemoveRows();
// }

// void MprisPlayersModel::playerUpdated()
// {
//     MprisPlayer* player = qobject_cast<MprisPlayer*>(QObject::sender());
//     Q_ASSERT(player);

//     auto i = m_players.indexOf(player->name());
//     dataChanged(index(i), index(i));
// }

// void MprisPlayersModel::pluginDestroyed()
// {
//     beginResetModel();
//     m_players.clear();
//     m_plugin = nullptr;
//     endResetModel();
// }

// void MprisPlayersModel::devicePluginsChanged()
// {
//     if (m_device == nullptr)
//         return;

//     // set plugin when it is reloaded
//     setPlugin(qobject_cast<MprisRemotePlugin*>(m_device->plugin(
//         QStringLiteral("SailfishConnect::MprisRemotePlugin"))));
// }

// } // namespace SailfishConnect
