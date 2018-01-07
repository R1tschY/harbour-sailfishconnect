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

#include "corelogging.h"

using namespace SailfishConnect;

struct KdeConnectPluginPrivate
{
    Device* m_device;
    QString m_pluginName;
    QSet<QString> m_outgoingCapabilties;
    SailfishConnectPluginConfig* m_config;
};

SailfishConnectPlugin::SailfishConnectPlugin(QObject* parent, const QVariantList& args)
    : QObject(parent)
    , d(new KdeConnectPluginPrivate)
{
    d->m_device = qvariant_cast< Device* >(args.at(0));
    d->m_pluginName = args.at(1).toString();
    d->m_outgoingCapabilties = args.at(2).toStringList().toSet();
    d->m_config = nullptr;
}

SailfishConnectPluginConfig* SailfishConnectPlugin::config() const
{
    //Create on demand, because not every plugin will use it
    if (!d->m_config) {
        d->m_config = new SailfishConnectPluginConfig(d->m_device->id(), d->m_pluginName);
    }
    return d->m_config;
}

SailfishConnectPlugin::~SailfishConnectPlugin()
{
    if (d->m_config) {
        delete d->m_config;
    }
}

const Device* SailfishConnectPlugin::device()
{
    return d->m_device;
}

Device const* SailfishConnectPlugin::device() const
{
    return d->m_device;
}

bool SailfishConnectPlugin::sendPackage(NetworkPackage& np) const
{
    if(!d->m_outgoingCapabilties.contains(np.type())) {
        qCWarning(coreLogger) << metaObject()->className() << "tried to send an unsupported package type" << np.type() << ". Supported:" << d->m_outgoingCapabilties;
        return false;
    }
     qCWarning(coreLogger) << metaObject()->className() << "sends" << np.type();
    return d->m_device->sendPackage(np);
}

QString SailfishConnectPlugin::dbusPath() const
{
    return {};
}
