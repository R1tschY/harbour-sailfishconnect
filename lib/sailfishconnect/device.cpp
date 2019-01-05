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

#include "device.h"

#ifdef interface // MSVC language extension, QDBusConnection uses this as a variable name
#undef interface
#endif

#include <QtGlobal>
#include <QSslCertificate>
#include <QSettings>
#include <QHostAddress>
#include <QRegularExpression>

#include "kdeconnectplugin.h"
#include "backend/devicelink.h"
#include "backend/lan/landevicelink.h"
#include "backend/linkprovider.h"
#include "networkpacket.h"
#include "kdeconnectconfig.h"
#include "corelogging.h"
#include "pluginloader.h"
#include <sailfishconnect/helper/cpphelper.h>

using namespace SailfishConnect;

struct DevicePrivate {
    const QString m_deviceId;
    QString m_deviceName;
    Device::DeviceType m_deviceType;
    int m_protocolVersion;

    QVector<DeviceLink*> m_deviceLinks;
    QHash<QString, KdeConnectPlugin*> m_plugins;

    //Capabilities stuff
    QMultiMap<QString, KdeConnectPlugin*> m_pluginsByIncomingCapability;
    QSet<QString> m_supportedPlugins;
    QSet<PairingHandler*> m_pairRequests;

    KdeConnectConfig* m_config;

    DevicePrivate()
        : m_protocolVersion(NetworkPacket::s_protocolVersion)
    { }

    DevicePrivate(KdeConnectConfig* config, const QString& id)
        : m_deviceId(Device::sanitizeDeviceId(id))
        , m_protocolVersion(NetworkPacket::s_protocolVersion) //We don't know it yet
        , m_config(config)
    { }
};

Device::Device()
    : d(new DevicePrivate())
{ }

Device::Device(QObject* parent, KdeConnectConfig* config, const QString& id)
    : Device(parent, config, id, QString(), QString())
{ }

Device::Device(QObject* parent, KdeConnectConfig* config, const QString& id, const QString& name, const QString& type)
    : QObject(parent)
    , d(new DevicePrivate(config, id))
{
    Q_ASSERT(config != nullptr);
    Q_ASSERT(!id.isEmpty());

    if (isTrusted()) {
        KdeConnectConfig::DeviceInfo info = d->m_config->getTrustedDevice(id);
        d->m_deviceName = info.deviceName;
        d->m_deviceType = str2type(info.deviceType);

        if (!name.isEmpty()) {
            setName(name);  // name may changed and should be emitted
        }
        if (!type.isEmpty()) {
            setType(type);
        }
    } else {
        d->m_deviceName = !name.isEmpty() ? name : QStringLiteral("unnamed");
        d->m_deviceType = !type.isEmpty() ? str2type(type) : Device::Unknown;
    }

    //Assume every plugin is supported until addLink is called and we can get the actual list
    d->m_supportedPlugins = PluginManager::instance()->getPluginList().toSet();

    connect(this, &Device::pairingError, this, [](const QString& info) {
        qWarning() << "Device pairing error" << info;
    });
}

Device::Device(QObject* parent, KdeConnectConfig* config, const NetworkPacket& identityPacket, DeviceLink* dl)
    : Device(
          parent,
          config,
          identityPacket.get<QString>(QStringLiteral("deviceId")),
          identityPacket.get<QString>(QStringLiteral("deviceName")),
          identityPacket.get<QString>(QStringLiteral("deviceType")))
{
    addLink(identityPacket, dl);
}

QString Device::id() const {
    return d->m_deviceId;
}

QString Device::name() const {
    return d->m_deviceName;
}

QString Device::dbusPath() const {
    return "/modules/kdeconnect/devices/" + id();
}

QString Device::type() const {
    return type2str(d->m_deviceType);
}

Device::~Device() = default;

bool Device::hasPlugin(const QString& name) const
{
    return d->m_plugins.contains(name);
}

QStringList Device::loadedPlugins() const
{
    return d->m_plugins.keys();
}

void Device::reloadPlugins()
{
    QHash<QString, KdeConnectPlugin*> newPluginMap, oldPluginMap = d->m_plugins;
    QMultiMap<QString, KdeConnectPlugin*> newPluginsByIncomingCapability;

    // Do not load any plugin for unpaired devices, nor useless loading them for
    // unreachable devices
    if (isTrusted() && isReachable()) {
        PluginManager* pluginManager = PluginManager::instance();

        for (const QString& pluginId : asConst(d->m_supportedPlugins)) {

            const bool pluginEnabled = isPluginEnabled(pluginId);
            if (pluginEnabled) {
                KdeConnectPlugin* plugin = d->m_plugins.take(pluginId);
                if (!plugin) {
                    plugin = pluginManager->instantiatePluginForDevice(
                                pluginId, this);
                }
                Q_ASSERT(plugin);

                const auto incomingCapabilities =
                        pluginManager->incomingCapabilities(pluginId);
                for (const QString& interface : incomingCapabilities) {
                    newPluginsByIncomingCapability.insert(interface, plugin);
                }

                newPluginMap[pluginId] = plugin;
            }
        }
    }

    const bool differentPlugins = oldPluginMap != newPluginMap;

    // Erase all left plugins in the original map (meaning that we don't want
    // them anymore, otherwise they would have been moved to the newPluginMap)
    qDeleteAll(d->m_plugins);
    d->m_plugins = newPluginMap;
    d->m_pluginsByIncomingCapability = newPluginsByIncomingCapability;

    if (differentPlugins) {
        Q_EMIT pluginsChanged();
    }
}

QString Device::pluginsConfigFile() const
{
    return KdeConnectConfig::instance()->deviceConfigDir(id()).absoluteFilePath(QStringLiteral("config"));
}

void Device::requestPair()
{
    if (isTrusted()) {
        Q_EMIT pairingError(tr("Already paired"));
        return;
    }

    if (!isReachable()) {
        Q_EMIT pairingError(tr("Device not reachable"));
        return;
    }

    for (DeviceLink* dl : asConst(d->m_deviceLinks)) {
        dl->userRequestsPair();
    }
}

void Device::unpair()
{
    for (DeviceLink* dl : asConst(d->m_deviceLinks)) {
        dl->userRequestsUnpair();
    }
    d->m_config->removeTrustedDevice(id());
    Q_EMIT trustedChanged(false);
}

void Device::pairStatusChanged(DeviceLink::PairStatus status)
{
    if (status == DeviceLink::NotPaired) {
        d->m_config->removeTrustedDevice(id());

        for (DeviceLink* dl : asConst(d->m_deviceLinks)) {
            if (dl != sender()) {
                dl->setPairStatus(DeviceLink::NotPaired);
            }
        }
    } else {
        d->m_config->addTrustedDevice(id(), name(), type());
    }

    reloadPlugins(); // Will load/unload plugins

    bool isTrusted = (status == DeviceLink::Paired);
    Q_EMIT trustedChanged(isTrusted);
    Q_ASSERT(isTrusted == this->isTrusted());
}

static bool lessThan(DeviceLink* p1, DeviceLink* p2)
{
    return p1->provider()->priority() > p2->provider()->priority();
}

void Device::addLink(const NetworkPacket& identityPacket, DeviceLink* link)
{
    qCDebug(coreLogger) << "Adding link to" << id() << "via" << link->provider();

    Q_ASSERT(sanitizeDeviceId(identityPacket.get<QString>(QStringLiteral("deviceId")))
             == d->m_deviceId);

    setName(identityPacket.get<QString>(QStringLiteral("deviceName")));
    setType(identityPacket.get<QString>(QStringLiteral("deviceType")));

    if (d->m_deviceLinks.contains(link))
        return;

    d->m_protocolVersion = identityPacket.get<int>(QStringLiteral("protocolVersion"), -1);
    if (d->m_protocolVersion != NetworkPacket::s_protocolVersion) {
        qCWarning(coreLogger) << d->m_deviceName << "- warning, device uses a different protocol version" << d->m_protocolVersion << "expected" << NetworkPacket::s_protocolVersion;
    }

    connect(link, &QObject::destroyed,
            this, &Device::linkDestroyed);

    d->m_deviceLinks.append(link);

    // Theoretically we will never add two links from the same provider (the provider should destroy
    // the old one before this is called), so we do not have to worry about destroying old links.
    // -- Actually, we should not destroy them or the provider will store an invalid ref!

    connect(link, &DeviceLink::receivedPacket,
            this, &Device::privateReceivedPacket);

    std::sort(d->m_deviceLinks.begin(), d->m_deviceLinks.end(), lessThan);

    const bool capabilitiesSupported = identityPacket.has(QStringLiteral("incomingCapabilities")) || identityPacket.has(QStringLiteral("outgoingCapabilities"));
    if (capabilitiesSupported) {
        const QSet<QString> outgoingCapabilities = identityPacket.get<QStringList>(QStringLiteral("outgoingCapabilities")).toSet()
                          , incomingCapabilities = identityPacket.get<QStringList>(QStringLiteral("incomingCapabilities")).toSet();

        d->m_supportedPlugins = PluginManager::instance()->pluginsForCapabilities(incomingCapabilities, outgoingCapabilities);
        qDebug() << "new plugins for" << d->m_deviceName << d->m_supportedPlugins << incomingCapabilities << outgoingCapabilities;
    } else {
        d->m_supportedPlugins = PluginManager::instance()->getPluginList().toSet();
    }

    reloadPlugins();

    if (d->m_deviceLinks.size() == 1) {
        Q_EMIT reachableChanged(true);
    }

    connect(link, &DeviceLink::pairStatusChanged, this, &Device::pairStatusChanged);
    connect(link, &DeviceLink::pairingRequest, this, &Device::addPairingRequest);
    connect(link, &DeviceLink::pairingRequestExpired, this, &Device::removePairingRequest);
    connect(link, &DeviceLink::pairingError, this, &Device::pairingError);
}

void Device::addPairingRequest(PairingHandler* handler)
{
    const bool wasEmpty = d->m_pairRequests.isEmpty();
    d->m_pairRequests.insert(handler);

    if (wasEmpty != d->m_pairRequests.isEmpty())
        Q_EMIT hasPairingRequestsChanged(!d->m_pairRequests.isEmpty());
}

void Device::removePairingRequest(PairingHandler* handler)
{
    const bool wasEmpty = d->m_pairRequests.isEmpty();
    d->m_pairRequests.remove(handler);

    if (wasEmpty != d->m_pairRequests.isEmpty())
        Q_EMIT hasPairingRequestsChanged(!d->m_pairRequests.isEmpty());
}

bool Device::hasPairingRequests() const
{
    return !d->m_pairRequests.isEmpty();
}

QString Device::pluginIconName(const QString &pluginName)
{
    return PluginManager::instance()->pluginIconUrl(pluginName);
}

void Device::acceptPairing()
{
    if (d->m_pairRequests.isEmpty())
        qWarning() << "no pair requests to accept!";

    //copying because the pairing handler will be removed upon accept
    const auto prCopy = d->m_pairRequests;
    for (auto ph: prCopy)
        ph->acceptPairing();
}

void Device::rejectPairing()
{
    if (d->m_pairRequests.isEmpty())
        qWarning() << "no pair requests to reject!";

    //copying because the pairing handler will be removed upon reject
    const auto prCopy = d->m_pairRequests;
    for (auto ph: prCopy)
        ph->rejectPairing();
}

void Device::linkDestroyed(QObject* o)
{
    removeLink(static_cast<DeviceLink*>(o));
}

void Device::removeLink(DeviceLink* link)
{
    d->m_deviceLinks.removeAll(link);

    qCDebug(coreLogger) << "RemoveLink" << d->m_deviceLinks.size() << "links remaining";

    if (d->m_deviceLinks.isEmpty()) {
        reloadPlugins();
        Q_EMIT reachableChanged(false);
    }
}

bool Device::sendPacket(NetworkPacket& np, JobManager* jobMgr)
{
    Q_ASSERT(np.type() != PACKET_TYPE_PAIR);
    Q_ASSERT(isTrusted());

    // Maybe we could block here any packet that is not an identity or a
    // pairing packet to prevent sending non encrypted data
    for (DeviceLink* dl : asConst(d->m_deviceLinks)) {
        if (dl->sendPacket(np, jobMgr)) return true;
    }

    return false;
}

void Device::privateReceivedPacket(const NetworkPacket& np)
{
    Q_ASSERT(np.type() != PACKET_TYPE_PAIR);
    if (isTrusted()) {
        const QList<KdeConnectPlugin*> plugins = d->m_pluginsByIncomingCapability.values(np.type());
        if (plugins.isEmpty()) {
            qWarning() << "discarding unsupported packet" << np.type() << "for" << name();
        }
        for (KdeConnectPlugin* plugin : plugins) {
            plugin->receivePacket(np);
        }
    } else {
        qCDebug(coreLogger) << "device" << name() << "not paired, ignoring packet" << np.type();
        unpair();
    }

}

bool Device::isTrusted() const
{
    return d->m_config->trustedDevices().contains(id());
}

QStringList Device::availableLinks() const
{
    QStringList sl;
    sl.reserve(d->m_deviceLinks.size());
    for (DeviceLink* dl : asConst(d->m_deviceLinks)) {
        sl.append(dl->provider()->name());
    }
    return sl;
}

bool Device::isReachable() const {
    return !d->m_deviceLinks.isEmpty();
}

void Device::cleanUnneededLinks() {
    if (isTrusted()) {
        return;
    }
    for(int i = 0; i < d->m_deviceLinks.size(); ) {
        DeviceLink* dl = d->m_deviceLinks[i];
        if (!dl->linkShouldBeKeptAlive()) {
            dl->deleteLater();
            d->m_deviceLinks.remove(i);
        } else {
            i++;
        }
    }
}

int Device::protocolVersion() {
    return d->m_protocolVersion;
}

QStringList Device::supportedPlugins() const {
    return d->m_supportedPlugins.toList();
}

QHostAddress Device::getLocalIpAddress() const
{
    for (DeviceLink* dl : d->m_deviceLinks) {
        LanDeviceLink* ldl = dynamic_cast<LanDeviceLink*>(dl);
        if (ldl) {
            return ldl->hostAddress();
        }
    }
    return QHostAddress::Null;
}

Device::DeviceType Device::str2type(const QString& deviceType) {
    if (deviceType == QLatin1String("desktop")) return Desktop;
    if (deviceType == QLatin1String("laptop")) return Laptop;
    if (deviceType == QLatin1String("smartphone") || deviceType == QLatin1String("phone")) return Phone;
    if (deviceType == QLatin1String("tablet")) return Tablet;
    if (deviceType == QLatin1String("tv")) return Tv;
    return Unknown;
}

QString Device::type2str(Device::DeviceType deviceType) {
    if (deviceType == Desktop) return QStringLiteral("desktop");
    if (deviceType == Laptop) return QStringLiteral("laptop");
    if (deviceType == Phone) return QStringLiteral("smartphone");
    if (deviceType == Tablet) return QStringLiteral("tablet");
    if (deviceType == Tv) return QStringLiteral("tv");
    return QStringLiteral("unknown");
}

QString Device::statusIconName() const
{
    return iconForStatus(isReachable(), isTrusted());
}

QString Device::iconName() const
{

    return iconForStatus(true, false);
}

QString Device::iconForStatus(bool reachable, bool trusted) const
{
    Device::DeviceType deviceType = d->m_deviceType;
    if (deviceType == Device::Unknown) {
        deviceType = Device::Phone; //Assume phone if we don't know the type
    } else if (deviceType == Device::Desktop) {
        deviceType = Device::Device::Laptop; // We don't have desktop icon yet
    }

    QString status = (reachable? (trusted? QStringLiteral("connected") : QStringLiteral("disconnected")) : QStringLiteral("trusted"));
    QString type = type2str(deviceType);

    return type+status;
}

QString Device::sanitizeDeviceId(const QString& deviceId)
{
    static QRegularExpression regexp("[^A-Za-z0-9_]");

    QString result = deviceId;
    result.replace(regexp, QLatin1String("_"));
    return result;
}

void Device::setName(const QString& name)
{
    if (d->m_deviceName != name) {
        d->m_deviceName = name;
        d->m_config->setDeviceProperty(
                    d->m_deviceId, QStringLiteral("name"), name);
        Q_EMIT nameChanged(name);
    }
}

void Device::setType(const QString &strtype)
{
    auto type = str2type(strtype);
    if (d->m_deviceType != type) {
        d->m_deviceType = type;
        d->m_config->setDeviceProperty(
                    d->m_deviceId, QStringLiteral("type"), type2str(type));
    }
}

KdeConnectPlugin* Device::plugin(const QString& pluginName) const
{
    return d->m_plugins[pluginName];
}

void Device::setPluginEnabled(const QString& pluginName, bool enabled)
{
    QSettings pluginStates(pluginsConfigFile(), QSettings::IniFormat);
    pluginStates.beginGroup("Plugins");

    const QString enabledKey = pluginName + QStringLiteral("Enabled");
    pluginStates.setValue(enabledKey, enabled);
    reloadPlugins();
}

bool Device::isPluginEnabled(const QString& pluginId) const
{
    const QString enabledKey = pluginId + QStringLiteral("Enabled");
    QSettings pluginStates(pluginsConfigFile(), QSettings::IniFormat);
    pluginStates.beginGroup("Plugins");

    return pluginStates.contains(enabledKey)
        ? pluginStates.value(enabledKey).toBool()
        : PluginManager::instance()->enabledByDefault(pluginId);
}

QString Device::encryptionInfo() const
{
    QString result;
    QCryptographicHash::Algorithm digestAlgorithm = QCryptographicHash::Algorithm::Sha1;

    QString localSha1 = QString::fromLatin1(
                d->m_config->certificate().digest(digestAlgorithm).toHex());
    for (int i = 2; i<localSha1.size(); i += 3) {
        localSha1.insert(i, ':'); // Improve readability
    }
    result += tr("SHA1 fingerprint of your device certificate is: %1\n").arg(localSha1);

    std::string remotePem = d->m_config->getDeviceProperty(
                id(), QStringLiteral("certificate")).toStdString();
    QSslCertificate remoteCertificate = QSslCertificate(QByteArray(remotePem.c_str(), (int)remotePem.size()));
    QString remoteSha1 = QString::fromLatin1(remoteCertificate.digest(digestAlgorithm).toHex());
    for (int i = 2; i < remoteSha1.size(); i += 3) {
        remoteSha1.insert(i, ':'); // Improve readability
    }
    result += tr("SHA1 fingerprint of remote device certificate is: %1\n").arg(remoteSha1);

    return result;
}

