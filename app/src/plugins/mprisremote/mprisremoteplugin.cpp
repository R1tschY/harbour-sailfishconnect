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

#include "mprisremoteplugin.h"

#include <QLoggingCategory>
#include <QDateTime>
#include <QSet>

#include <sailfishconnect/device.h>
#include <sailfishconnect/helper/cpphelper.h>

namespace SailfishConnect {

static Q_LOGGING_CATEGORY(logger, "kdeconnect.plugin.mprisremote")

static const QString PACKET_TYPE_MPRIS_REQUEST =
        QStringLiteral("kdeconnect.mpris.request");

static const QString PACKET_TYPE_MPRIS =
        QStringLiteral("kdeconnect.mpris");

// MprisPlayer

MprisPlayer::MprisPlayer(MprisRemotePlugin *parent, const QString& name)
    : QObject(parent), m_parent(parent), m_player(name),
      m_isSpotify(m_player.toLower() == QLatin1String("spotify"))
{ }

qlonglong MprisPlayer::position() const
{
    if (m_isPlaying && m_lastPosition >= 0) {
        return m_lastPosition +
                QDateTime::currentMSecsSinceEpoch() - m_lastPositionTime;
    } else {
        return m_lastPosition;
    }
}

bool MprisPlayer::seekAllowed() const
{
    return m_seekAllowed && m_length >= 0 && m_lastPosition >= 0
            && !isSpotify();
}

void MprisPlayer::setVolume(int value)
{
    if (setVolumeAllowed()) {
        m_parent->sendCommand(m_player, "setVolume", value);

        m_volume = value;

        emit propertiesChanged();
    }
}

void MprisPlayer::setPosition(int value)
{
    if (seekAllowed()) {
        m_parent->sendCommand(m_player, "SetPosition", value);

        m_lastPosition = value;
        m_lastPositionTime = QDateTime::currentMSecsSinceEpoch();

        emit propertiesChanged();
    }
}

void MprisPlayer::receivePacket(const NetworkPacket &np)
{
    m_currentSong =
            np.get<QString>(QStringLiteral("nowPlaying"), m_currentSong);
    m_title =
            np.get<QString>(QStringLiteral("title"), m_title);
    m_artist =
            np.get<QString>(QStringLiteral("artist"), m_artist);
    m_album =
            np.get<QString>(QStringLiteral("album"), m_album);
    m_albumArtUrl =
            np.get<QString>(QStringLiteral("albumArtUrl"), m_albumArtUrl);
    m_length =
            np.get<qint64>(QStringLiteral("length"), m_length);
    m_isPlaying =
            np.get<bool>(QStringLiteral("isPlaying"), m_isPlaying);
    m_playAllowed =
            np.get<bool>(QStringLiteral("canPlay"), m_playAllowed);
    m_pauseAllowed =
            np.get<bool>(QStringLiteral("canPause"), m_pauseAllowed);
    m_goNextAllowed =
            np.get<bool>(QStringLiteral("canGoNext"), m_goNextAllowed);
    m_goPreviousAllowed =
            np.get<bool>(QStringLiteral("canGoPrevious"), m_goPreviousAllowed);
    m_seekAllowed =
            np.get<bool>(QStringLiteral("canSeek"), m_seekAllowed);

    if (np.has(QStringLiteral("pos")) && !isSpotify()) {
        m_lastPosition = np.get<qint64>(QStringLiteral("pos"), m_lastPosition);
        m_lastPositionTime = QDateTime::currentMSecsSinceEpoch();
    }

    emit propertiesChanged();
}

void MprisPlayer::playPause()
{
    if (pauseAllowed() || playAllowed()) {
        m_parent->sendCommand(m_player, "action", "PlayPause");
    }
}

void MprisPlayer::play()
{
    if (playAllowed()) {
        m_parent->sendCommand(m_player, "action", "Play");
    }
}

void MprisPlayer::pause()
{
    if (pauseAllowed()) {
        m_parent->sendCommand(m_player, "action", "Pause");
    }
}

void MprisPlayer::stop()
{
    m_parent->sendCommand(m_player, "action", "Stop");
}

void MprisPlayer::previous()
{
    if (goPreviousAllowed()) {
        m_parent->sendCommand(m_player, "action", "Previous");
    }
}

void MprisPlayer::next()
{
    if (goNextAllowed()) {
        m_parent->sendCommand(m_player, "action", "Next");
    }
}

void MprisPlayer::seek(int value)
{
    if (seekAllowed()) {
        m_parent->sendCommand(m_player, "seek", value);
    }
}

// MprisRemotePlugin

MprisRemotePlugin::MprisRemotePlugin(Device* device,
                                     const QString &name,
                                     const QSet<QString> &outgoingCapabilities)
    : KdeConnectPlugin(device, name, outgoingCapabilities)
{
    requestPlayerList();
}

bool MprisRemotePlugin::receivePacket(const NetworkPacket& np)
{
    if (np.get<bool>("transferringAlbumArt", false)) {
        // TODO: not supported yet
        return true;
    }

    if (np.has("player")) {
        MprisPlayer* player = m_players.value(
                    np.get<QString>(QStringLiteral("player")), nullptr);
        if (player) {
            player->receivePacket(np);
        }
    }

    m_supportAlbumArtPayload = np.get<bool>(
                QStringLiteral("supportAlbumArtPayload"),
                m_supportAlbumArtPayload);

    if (np.has(QStringLiteral("playerList"))) {
        QSet<QString> newPlayerList(
                    np.get<QStringList>(QStringLiteral("playerList")).toSet());
        QSet<QString> oldPlayerList(m_players.keys().toSet());

        QSet<QString> addedPlayers = newPlayerList;
        addedPlayers.subtract(oldPlayerList);

        oldPlayerList.subtract(newPlayerList);

        for (auto addedPlayer : addedPlayers) {
            m_players.insert(addedPlayer, new MprisPlayer(this, addedPlayer));
            requestPlayerStatus(addedPlayer);
        }
        for (auto removedPlayer : oldPlayerList) {
            m_players[removedPlayer]->deleteLater();
            m_players.remove(removedPlayer);
        }

        for (auto addedPlayer : addedPlayers) {
            emit playerAdded(addedPlayer);
        }
        for (auto removedPlayer : oldPlayerList) {
            emit playerRemoved(removedPlayer);
        }
    }

    return true;
}

void MprisRemotePlugin::sendCommand(
        const QString& player, const QString& method, const QString& value)
{
    NetworkPacket np(PACKET_TYPE_MPRIS_REQUEST);
    np.set("player", player);
    np.set(method, value);
    sendPacket(np);
}

void MprisRemotePlugin::sendCommand(
        const QString& player, const QString& method, int value)
{
    NetworkPacket np(PACKET_TYPE_MPRIS_REQUEST);
    np.set("player", player);
    np.set(method, value);
    sendPacket(np);
}

QStringList MprisRemotePlugin::players() const
{
    QStringList result = m_players.keys();
    result.sort();
    return result;
}

MprisPlayer* MprisRemotePlugin::player(const QString &name) const
{
    return m_players.value(name);
}

void MprisRemotePlugin::requestPlayerList()
{
    NetworkPacket np(PACKET_TYPE_MPRIS_REQUEST);
    np.set("requestPlayerList", true);
    sendPacket(np);
}

void MprisRemotePlugin::requestPlayerStatus(const QString& player)
{
    NetworkPacket np(PACKET_TYPE_MPRIS_REQUEST);
    np.set("player", player);
    np.set("requestNowPlaying", true);
    np.set("requestVolume", true);
    sendPacket(np);
}

QString MprisRemotePluginFactory::name() const
{
    return tr("Multimedia controls");
}

QString MprisRemotePluginFactory::description() const
{
    return tr("Control your media player from remote.");
}

QString MprisRemotePluginFactory::iconUrl() const
{
    return "image://theme/icon-m-media";
}

} // namespace SailfishConnect

Q_IMPORT_PLUGIN(MprisRemotePluginFactory)
