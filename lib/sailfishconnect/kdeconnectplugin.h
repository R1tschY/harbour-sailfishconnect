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

#ifndef KDECONNECTPLUGIN_H
#define KDECONNECTPLUGIN_H

#include <memory>
#include <QObject>
#include <QSet>
#include <QScopedPointer>


struct KdeConnectPluginPrivate;
class KdeConnectPlugin;
class Device;
class NetworkPacket;
class SailfishConnectPluginConfig;

namespace SailfishConnect {
class JobManager;
} // namespace SailfishConnect


class SailfishConnectPluginFactory
{
public:
    virtual ~SailfishConnectPluginFactory() = default;

    virtual KdeConnectPlugin* create(
            Device* device,
            const QString& name,
            const QSet<QString>& outgoingCapabilities) = 0;

    virtual QString name() const = 0;
    virtual QString description() const = 0;
    virtual QString iconUrl() const = 0;

    virtual void registerTypes();
};

#define SailfishConnectPlugin_iid "SailfishConnect.Plugin"
Q_DECLARE_INTERFACE(SailfishConnectPluginFactory, SailfishConnectPlugin_iid)


template<typename T>
class SailfishConnectPluginFactory_ :
        public QObject, public SailfishConnectPluginFactory
{
public:
    using FactoryBaseType = SailfishConnectPluginFactory_<T>;

    KdeConnectPlugin* create(
            Device* device,
            const QString& name,
            const QSet<QString>& outgoingCapabilities) override
    {
        return new T(device, name, outgoingCapabilities);
    }
};



class KdeConnectPlugin : public QObject
{
    Q_OBJECT

public:
    KdeConnectPlugin(
            Device* device, const QString& id,
            const QSet<QString>& outgoingCapabilities);
    ~KdeConnectPlugin();

    QString id() const;

    const Device* device() const;

    bool sendPacket(NetworkPacket& np, SailfishConnect::JobManager *jobMgr = nullptr) const;

    SailfishConnectPluginConfig* config() const;

public Q_SLOTS:
    /**
     * Returns true if it has handled the packet in some way
     * device.sendPacket can be used to send an answer back to the device
     */
    virtual bool receivePacket(const NetworkPacket& np) = 0;

    /**
     * This method will be called when a device is connected to this computer.
     * The plugin could be loaded already, but there is no guarantee we will be able to reach the device until this is called.
     */
    virtual void connected() { }

private:
    QScopedPointer<KdeConnectPluginPrivate> d;
};

#endif
