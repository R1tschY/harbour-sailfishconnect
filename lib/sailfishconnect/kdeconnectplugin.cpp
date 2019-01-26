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

#include "kdeconnectplugin.h"

#include <memory>
#include "corelogging.h"
#include "device.h"
#include "networkpacket.h"
#include "kdeconnectpluginconfig.h"

using namespace SailfishConnect;

struct KdeConnectPluginPrivate
{
    Device* m_device;
    QString m_pluginId;
    QSet<QString> m_outgoingCapabilties;
    std::unique_ptr<SailfishConnectPluginConfig> m_config = nullptr;
};

KdeConnectPlugin::KdeConnectPlugin(Device* device,
        const QString &id,
        const QSet<QString> &outgoingCapabilities)
    : QObject(device)
    , d(new KdeConnectPluginPrivate)
{
    d->m_device = device;
    d->m_pluginId = id;
    d->m_outgoingCapabilties = outgoingCapabilities;
}

QString KdeConnectPlugin::id() const
{
    return d->m_pluginId;
}

KdeConnectPlugin::~KdeConnectPlugin() = default;

SailfishConnectPluginConfig* KdeConnectPlugin::config() const
{
    // Create on demand, because not every plugin will use it
    if (!d->m_config) {
        d->m_config.reset(new SailfishConnectPluginConfig(
                              d->m_device->id(), d->m_pluginId));
    }
    return d->m_config.get();
}

const Device* KdeConnectPlugin::device() const
{
    return d->m_device;
}

bool KdeConnectPlugin::sendPacket(NetworkPacket& np, JobManager* jobMgr) const
{
    if (!d->m_outgoingCapabilties.contains(np.type())) {
        qCWarning(coreLogger).nospace()
                << metaObject()->className()
                << " tried to send an unsupported packet type " << np.type()
                << ". Supported: " << d->m_outgoingCapabilties;
        return false;
    }
    qCDebug(coreLogger) << metaObject()->className() << "sends" << np.type();
    qCDebug(coreLogger) << "data:" << np.serialize().data();
    return d->m_device->sendPacket(np, jobMgr);
}


void SailfishConnectPluginFactory::registerTypes()
{

}
