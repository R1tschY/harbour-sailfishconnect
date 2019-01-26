/**
 * Copyright 2013 Albert Vaca <albertvaka@gmail.com>
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

#include "daemon.h"

#include <QNetworkAccessManager>
#include <QDebug>
#include <QPointer>

#include "corelogging.h"
#include "kdeconnectconfig.h"
#include "networkpacket.h"

#include "backend/lan/lanlinkprovider.h"
//#include "backend/loopback/loopbacklinkprovider.h"
#include "device.h"
#include "backend/devicelink.h"
#include "backend/linkprovider.h"
#include "corelogging.h"
#include "helper/cpphelper.h"
#include "io/jobmanager.h"
#include "kdeconnectconfig.h"
#include "systeminfo.h"

using namespace SailfishConnect;

static Daemon* s_instance = nullptr;

struct DaemonPrivate
{
    //Different ways to find devices and connect to them
    QList<LinkProvider*> m_linkProviders;

    //Every known device
    QHash<QString, Device*> m_devices;

    QSet<QString> m_discoveryModeAcquisitions;

    KdeConnectConfig m_config;
    JobManager* m_jobManager;

    DaemonPrivate(std::unique_ptr<SystemInfo> systemInfo)
    : m_config(std::move(systemInfo))
    { }
};

Daemon* Daemon::instance()
{
    Q_ASSERT(s_instance != nullptr);
    return s_instance;
}

Daemon::Daemon(std::unique_ptr<SystemInfo> systemInfo, QObject *parent)
    : Daemon(std::move(systemInfo), standardLinkProviders(), parent)
{ }

Daemon::Daemon(std::unique_ptr<SystemInfo> systemInfo, QList<LinkProvider*> link_providers, QObject* parent)
    : QObject(parent)
    , d(new DaemonPrivate(std::move(systemInfo)))
{
    Q_ASSERT(s_instance == nullptr);
    s_instance = this;
    qCDebug(coreLogger) << "KdeConnect daemon starting";

    d->m_jobManager = new JobManager(this);

    // Loading config
    if (!d->m_config.valid()) {
        qCCritical(coreLogger) << "no valid configuation. aborting!";
        return;
    }

    d->m_linkProviders = link_providers;

    //Read remebered paired devices
    const QStringList& list = d->m_config.trustedDevices();
    for (const QString& id : list) {
        addDevice(new Device(this, &d->m_config, id));
    }

    //Listen to new devices
    for (LinkProvider* a : asConst(d->m_linkProviders)) {
        connect(a, &LinkProvider::onConnectionReceived,
                this, &Daemon::onNewDeviceLink);
        a->onStart();
    }

    qCDebug(coreLogger) << "KdeConnect daemon started";
}

void Daemon::acquireDiscoveryMode(const QString& key)
{
    bool oldState = d->m_discoveryModeAcquisitions.isEmpty();

    d->m_discoveryModeAcquisitions.insert(key);

    if (oldState != d->m_discoveryModeAcquisitions.isEmpty()) {
        forceOnNetworkChange("discovery mode started with key " + key);
    }
}

void Daemon::releaseDiscoveryMode(const QString& key)
{
    bool oldState = d->m_discoveryModeAcquisitions.isEmpty();

    d->m_discoveryModeAcquisitions.remove(key);

    if (oldState != d->m_discoveryModeAcquisitions.isEmpty()) {
        cleanDevices();
    }
}

void Daemon::removeDevice(Device* device)
{
    d->m_devices.remove(device->id());
    device->deleteLater();
    Q_EMIT deviceRemoved(device->id());
    Q_EMIT deviceListChanged();
}

void Daemon::cleanDevices()
{
    for (Device* device : asConst(d->m_devices)) {
        if (device->isTrusted()) {
            continue;
        }
        device->cleanUnneededLinks();
        //If there are no links remaining
        if (!device->isReachable()) {
            removeDevice(device);
        }
    }
}

QList<LinkProvider *> Daemon::standardLinkProviders()
{
    QList<LinkProvider*> result;
    result.append(new LanLinkProvider());
#ifdef KDECONNECT_BLUETOOTH
    result.insert(new BluetoothLinkProvider());
#endif
    return result;
}

void Daemon::forceOnNetworkChange(const QString& reason)
{
    qCDebug(coreLogger)
            << "Sending onNetworkChange to"
            << d->m_linkProviders.size() << "LinkProviders";
    for (LinkProvider* a : asConst(d->m_linkProviders)) {
        a->onNetworkChange(reason);
    }
}

Device* Daemon::getDevice(const QString& deviceId)
{
    for (Device* device : asConst(d->m_devices)) {
        if (device->id() == deviceId) {
            return device;
        }
    }
    return Q_NULLPTR;
}

QStringList Daemon::devices(bool onlyReachable, bool onlyTrusted)
{
    QStringList ret;
    for (Device* device : asConst(d->m_devices)) {
        if (onlyReachable && !device->isReachable()) continue;
        if (onlyTrusted && !device->isTrusted()) continue;
        ret.append(device->id());
    }
    return ret;
}

void Daemon::onNewDeviceLink(const NetworkPacket& identityPacket, DeviceLink* dl)
{
    const QString& id = identityPacket.get<QString>(QStringLiteral("deviceId"));

    qCDebug(coreLogger) << "Device discovered" << id << "via" << dl->provider()->name();

    if (d->m_devices.contains(id)) {
        qCDebug(coreLogger) << "It is a known device" << identityPacket.get<QString>(QStringLiteral("deviceName"));
        Device* device = d->m_devices[id];
        bool wasReachable = device->isReachable();
        device->addLink(identityPacket, dl);
        if (!wasReachable) {
            Q_EMIT deviceVisibilityChanged(id, true);
            Q_EMIT deviceListChanged();
        }
    } else {
        qCDebug(coreLogger) << "It is a new device" << identityPacket.get<QString>(QStringLiteral("deviceName"));
        Device* device = new Device(this, &d->m_config, identityPacket, dl);

        //we discard the connections that we created but it's not paired.
        if (!isDiscoveringDevices() && !device->isTrusted() && !dl->linkShouldBeKeptAlive()) {
            qCDebug(coreLogger) << "device discarded :(";
            device->deleteLater();
        } else {
            addDevice(device);
        }
    }
}

void Daemon::onDeviceStatusChanged()
{
    Device* device = (Device*)sender();

    qCDebug(coreLogger) << "Device" << device->name() << "status changed. Reachable:" << device->isReachable() << ". Trusted: " << device->isTrusted();

    Q_EMIT deviceVisibilityChanged(device->id(), device->isReachable());
    Q_EMIT deviceListChanged();
}

void Daemon::setAnnouncedName(const QString& name)
{
    if (name != d->m_config.name()) {
        qCDebug(coreLogger()) << "Announcing name";
        d->m_config.setName(name);
        forceOnNetworkChange("device name changed");
        Q_EMIT announcedNameChanged(name);
    }
}

QString Daemon::announcedName()
{
    return d->m_config.name();
}

QNetworkAccessManager* Daemon::networkAccessManager()
{
    static QPointer<QNetworkAccessManager> manager; // TODO: make member
    if (!manager) {
        manager = new QNetworkAccessManager(this);
    }
    return manager;
}

QList<Device*> Daemon::devicesList() const
{
    return d->m_devices.values();
}

KdeConnectConfig *Daemon::config()
{
    return &d->m_config;
}

JobManager *Daemon::jobManager()
{
    return d->m_jobManager;
}

bool Daemon::isDiscoveringDevices() const
{
    return !d->m_discoveryModeAcquisitions.isEmpty();
}

QString Daemon::deviceIdByName(const QString& name)
{
    for (Device* device : asConst(d->m_devices)) {
        if (device->name() == name && device->isTrusted())
            return device->id();
    }
    return {};
}

void Daemon::addDevice(Device* device)
{
    const QString id = device->id();
    connect(device, &Device::reachableChanged, this, &Daemon::onDeviceStatusChanged);
    connect(device, &Device::trustedChanged, this, &Daemon::onDeviceStatusChanged);
    connect(device, &Device::hasPairingRequestsChanged, this, &Daemon::pairingRequestsChanged);
    connect(device, &Device::hasPairingRequestsChanged, this, [this, device](bool hasPairingRequests) {
        if (hasPairingRequests)
            askPairingConfirmation(device);
    } );
    d->m_devices[id] = device;

    Q_EMIT deviceAdded(id);
    Q_EMIT deviceListChanged();
}

QStringList Daemon::pairingRequests() const
{
    QStringList ret;
    for (Device* dev : asConst(d->m_devices)) {
        if (dev->hasPairingRequests())
            ret += dev->id();
    }
    return ret;
}

Daemon::~Daemon() {
    Q_ASSERT(s_instance == this);
    s_instance = nullptr;
}

QString Daemon::selfId() const
{
    return d->m_config.deviceId();
}
