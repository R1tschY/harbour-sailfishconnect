/*
 * Copyright 2013 Albert Vaca <albertvaka@gmail.com>
 * Copyright 2019 Richard Liebscher <richard.liebscher@gmail.com>
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

#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include <QScopedPointer>

#include "backend/devicelink.h"

class DeviceLink;
class KdeConnectPlugin;
class KdeConnectConfig;
class QString;
class NetworkPacket;
class QStringList;
class PairingHandler;

namespace SailfishConnect {
class JobManager;
} // namespace SailfishConnect

class Device
    : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString type READ type CONSTANT)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString iconName READ iconName CONSTANT)
    Q_PROPERTY(QString statusIconName READ statusIconName)
    Q_PROPERTY(bool isReachable READ isReachable NOTIFY reachableChanged)
    Q_PROPERTY(bool isTrusted READ isTrusted NOTIFY trustedChanged)
    Q_PROPERTY(QStringList loadedPlugins READ loadedPlugins NOTIFY pluginsChanged)
    Q_PROPERTY(QStringList supportedPlugins READ supportedPlugins NOTIFY pluginsChanged)
    Q_PROPERTY(bool hasPairingRequests READ hasPairingRequests NOTIFY hasPairingRequestsChanged)
    Q_PROPERTY(bool waitsForPairing READ waitsForPairing NOTIFY waitsForPairingChanged)

public:

    enum DeviceType {
        Unknown,
        Desktop,
        Laptop,
        Phone,
        Tablet,
        Tv,
    };

    Device();

    /**
     * Restores the @p device from the saved configuration
     *
     * We already know it but we need to wait for an incoming DeviceLink to communicate
     */
    Device(QObject* parent, KdeConnectConfig* config, const QString& id);

    /**
     * Device known via an incoming connection sent to us via a devicelink.
     *
     * We know everything but we don't trust it yet
     */
    Device(QObject* parent, KdeConnectConfig *config, const QString& id, const QString& name, const QString &type);

    Device(QObject* parent, KdeConnectConfig *config, const NetworkPacket& np, DeviceLink* dl);

    ~Device() override;

    QString id() const;
    QString name() const;
    QString dbusPath() const;
    QString type() const;
    QString iconName() const;
    QString statusIconName() const;
    Q_SCRIPTABLE QString encryptionInfo() const;

    //Add and remove links
    void addLink(const NetworkPacket& identityPacket, DeviceLink*);
    void removeLink(DeviceLink*);

    bool isTrusted() const;
    bool hasPairingRequests() const;
    bool waitsForPairing() const;

    Q_SCRIPTABLE QStringList availableLinks() const;
    bool isReachable() const;

    Q_SCRIPTABLE QStringList loadedPlugins() const;
    Q_SCRIPTABLE bool hasPlugin(const QString& name) const;

    Q_SCRIPTABLE QString pluginsConfigFile() const;

    Q_SCRIPTABLE KdeConnectPlugin* plugin(const QString& pluginName) const;
    Q_SCRIPTABLE void setPluginEnabled(const QString& pluginName, bool enabled);
    Q_SCRIPTABLE bool isPluginEnabled(const QString& pluginName) const;

    void cleanUnneededLinks();

    int protocolVersion();
    QStringList supportedPlugins() const;

    /**
     * @brief sanitize device id
     *
     * The device id has to be a valid ascii identifer (/[A-Za-z0-9_]+/). This
     * function replaces all not allowed characters with `_`. Ids containing
     * characters that are not allowed as dbus paths would make app crash.
     *
     * @param deviceId a non-sanitized device id
     * @return sanitized device id
     */
    static QString sanitizeDeviceId(const QString& deviceId);

public Q_SLOTS:
    ///sends a @p np packet to the device
    ///virtual for testing purposes.
    virtual bool sendPacket(NetworkPacket& np, SailfishConnect::JobManager *jobMgr = nullptr);

    //Dbus operations
public Q_SLOTS:
    Q_SCRIPTABLE void requestPair(); //to all links
    Q_SCRIPTABLE void unpair(); //from all links
    Q_SCRIPTABLE void reloadPlugins(); //from kconf

    Q_SCRIPTABLE void acceptPairing();
    Q_SCRIPTABLE void rejectPairing();

    /**
     * @brief just to be compatible with KDE Connect Interface
     * @see PluginManager::pluginIconUrl
     */
    Q_SCRIPTABLE QString pluginIconName(const QString& pluginName);

private Q_SLOTS:
    void privateReceivedPacket(const NetworkPacket& np);
    void linkDestroyed(QObject* o);
    void pairStatusChanged(DeviceLink::PairStatus current);
    void addPairingRequest(PairingHandler* handler);
    void removePairingRequest(PairingHandler* handler);

Q_SIGNALS:
    Q_SCRIPTABLE void pluginsChanged();
    Q_SCRIPTABLE void reachableChanged(bool reachable);
    Q_SCRIPTABLE void trustedChanged(bool trusted);
    Q_SCRIPTABLE void pairingError(const QString& error);
    Q_SCRIPTABLE void nameChanged(const QString& name);

    Q_SCRIPTABLE void hasPairingRequestsChanged(bool hasPairingRequests);
    Q_SCRIPTABLE void waitsForPairingChanged(bool waitsForPairing);

private: //Methods
    static DeviceType str2type(const QString& deviceType);
    static QString type2str(DeviceType deviceType);

    void setName(const QString& name);
    void setType(const QString& type);
    void setWaitsForPairing(bool value);
    QString iconForStatus(bool reachable, bool paired) const;

private:
    QScopedPointer<struct DevicePrivate> d;
};

Q_DECLARE_METATYPE(Device*)

#endif // DEVICE_H
