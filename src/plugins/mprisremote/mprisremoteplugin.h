#ifndef MPRISREMOTEPLUGIN_H
#define MPRISREMOTEPLUGIN_H

#include <QObject>
#include <QtPlugin>
#include <QFlags>
#include <QMap>

#include "../../core/kdeconnectplugin.h"

namespace SailfishConnect {

class MprisRemotePlugin;

class MprisPlayer : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY propertiesChanged)
    Q_PROPERTY(QString currentSong READ currentSong NOTIFY propertiesChanged)
    Q_PROPERTY(QString title READ title NOTIFY propertiesChanged)
    Q_PROPERTY(QString artist READ artist NOTIFY propertiesChanged)
    Q_PROPERTY(QString album READ album NOTIFY propertiesChanged)
    Q_PROPERTY(QString albumArtUrl READ albumArtUrl NOTIFY propertiesChanged)
    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY propertiesChanged)
    Q_PROPERTY(qint64 length READ length NOTIFY propertiesChanged)
    Q_PROPERTY(qint64 position READ position WRITE setPosition)
    Q_PROPERTY(bool playAllowed READ playAllowed NOTIFY propertiesChanged)
    Q_PROPERTY(bool pauseAllowed READ pauseAllowed NOTIFY propertiesChanged)
    Q_PROPERTY(bool goNextAllowed READ goNextAllowed NOTIFY propertiesChanged)
    Q_PROPERTY(bool goPreviousAllowed READ goPreviousAllowed NOTIFY propertiesChanged)
    Q_PROPERTY(bool seekAllowed READ seekAllowed NOTIFY propertiesChanged)
    Q_PROPERTY(bool setVolumeAllowed READ setVolumeAllowed NOTIFY propertiesChanged)

public:
    MprisPlayer(MprisRemotePlugin* parent, const QString& name);

    QString name() const { return m_player; }
    bool isPlaying() const { return m_isPlaying; }
    QString currentSong() const { return m_currentSong; }
    QString title() const { return m_title; }
    QString artist() const { return m_artist; }
    QString album() const { return m_album; }
    QString albumArtUrl() const { return m_albumArtUrl; }
    int volume() const { return m_volume; }
    qlonglong length() const { return m_length; }
    qlonglong position() const;
    bool playAllowed() const { return m_playAllowed; }
    bool pauseAllowed() const { return m_pauseAllowed; }
    bool goNextAllowed() const { return m_goNextAllowed; }
    bool goPreviousAllowed() const { return m_goPreviousAllowed; }
    bool seekAllowed() const;
    bool setVolumeAllowed() const { return !isSpotify(); }

    void setVolume(int value);
    void setPosition(int value);

    void receivePackage(const NetworkPackage& np);

public slots:
    Q_SCRIPTABLE void playPause();
    Q_SCRIPTABLE void play();
    Q_SCRIPTABLE void pause();
    Q_SCRIPTABLE void stop();
    Q_SCRIPTABLE void previous();
    Q_SCRIPTABLE void next();
    Q_SCRIPTABLE void seek(int value);

signals:
    void propertiesChanged();

private:
    MprisRemotePlugin* m_parent;

    const QString m_player;
    const bool m_isSpotify = false;
    bool m_isPlaying = false;
    bool m_playAllowed = true;
    bool m_pauseAllowed = true;
    bool m_goNextAllowed = true;
    bool m_goPreviousAllowed = true;
    bool m_seekAllowed = true;
    int m_volume = -1;
    qint64 m_length = -1;
    qint64 m_lastPosition = 0;
    qint64 m_lastPositionTime = 0;
    QString m_currentSong;
    QString m_title;
    QString m_artist;
    QString m_album;
    QString m_albumArtUrl;

    /**
     * @brief detect Spotify for workarounds
     */
    bool isSpotify() const
    {
        return m_isSpotify;
    }
};


class MprisRemotePlugin : public KdeConnectPlugin
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.kdeconnect.device.mprisremote")
    Q_PROPERTY(QStringList players READ players)
public:
    explicit MprisRemotePlugin(
            Device* device,
            const QString &name,
            const QSet<QString> &outgoingCapabilities);


    bool receivePackage(const NetworkPackage& np) override;

    void sendCommand(
            const QString& player, const QString& method, const QString& value);
    void sendCommand(
            const QString& player, const QString& method, int value);

    QStringList players() const;
    Q_SCRIPTABLE MprisPlayer* player(const QString& name) const;

signals:
    void playerAdded(const QString& player);
    void playerRemoved(const QString& player);

private:
    QMap<QString, MprisPlayer*> m_players;

    bool m_supportAlbumArtPayload = false;

    void requestPlayerList();
    void requestPlayerStatus(const QString& player);
};

class MprisRemotePluginFactory :
        public SailfishConnectPluginFactory_<MprisRemotePlugin>
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID SailfishConnectPlugin_iid FILE "metadata.json")
    Q_INTERFACES(SailfishConnectPluginFactory)
public:
    using FactoryBaseType::FactoryBaseType;

    QString name() const override;
    QString description() const override;
    QString iconUrl() const override;
};

} // namespace SailfishConnect

#endif
