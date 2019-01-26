/**
 * Copyright 2015 Albert Vaca <albertvaka@gmail.com>
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

#ifndef KDECONNECTCONFIG_H
#define KDECONNECTCONFIG_H

#include <memory>
#include <QCoreApplication>

class QSslCertificate;
class QSslKey;
class QDir;

namespace SailfishConnect {
class SystemInfo;
} // namespace SailfishConnect

class KdeConnectConfig
{
    Q_DECLARE_TR_FUNCTIONS(KdeConnectConfig)
public:
    KdeConnectConfig(std::unique_ptr<SailfishConnect::SystemInfo> systemInfo);

    struct DeviceInfo {
        QString deviceName;
        QString deviceType;
    };

    static KdeConnectConfig* instance();

    /*
     * Our own info
     */

    QString deviceId() const;
    QString name() const;
    QString defaultName() const;
    QString deviceType() const;

    QString privateKeyPath() const;

    QString certificatePath() const;
    QSslCertificate certificate() const;

    QString configPath() const;
    QString trustedDevicesConfigPath() const;

    void setName(const QString& name);

    bool valid() const;

    /*
     * Trusted devices
     */

    QStringList trustedDevices() const; //list of ids
    void removeTrustedDevice(const QString& id);
    void addTrustedDevice(const QString& id, const QString& name, const QString& type);
    KdeConnectConfig::DeviceInfo getTrustedDevice(const QString& id) const;

    void setDeviceProperty(const QString& deviceId, const QString& name, const QString& value);
    QString getDeviceProperty(const QString& deviceId, const QString& name, const QString& defaultValue = QString()) const;

    /*
     * Paths for config files, there is no guarantee the directories already exist
     */
    // TODO: return QString
    QDir baseConfigDir() const;
    QDir deviceConfigDir(const QString& deviceId) const;
    QDir pluginConfigDir(const QString& deviceId, const QString& pluginName) const; //Used by KdeConnectPluginConfig

private:
    struct KdeConnectConfigPrivate* d;

    void createBaseConfigDir();
    void createCertificate();
    void createDeviceId();
    void createName();
};

#endif
