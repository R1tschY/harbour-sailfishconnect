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

#include "kdeconnectconfig.h"

#include <QFile>
#include <QDebug>
#include <QFileInfo>
#include <QUuid>
#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QHostInfo>
#include <QSettings>
#include <QSslCertificate>
#include <QSslKey>

#include "corelogging.h"
#include "daemon.h"
#include "../utils/sslhelper.h"

using namespace SailfishConnect;

struct KdeConnectConfigPrivate {

    QSslKey m_privateKey;
    QSslKey m_publicKey;
    QSslCertificate m_certificate;

    QSettings* m_config;
    QSettings* m_trustedDevices;

};

KdeConnectConfig* KdeConnectConfig::instance()
{
    static KdeConnectConfig* kcc = new KdeConnectConfig();
    return kcc;
}

KdeConnectConfig::KdeConnectConfig()
    : d(new KdeConnectConfigPrivate)
{
    //Make sure base directory exists
    QDir().mkpath(baseConfigDir().path());

    //.config/kdeconnect/config
    d->m_config = new QSettings(baseConfigDir().absoluteFilePath(QStringLiteral("config")), QSettings::IniFormat);
    d->m_trustedDevices = new QSettings(baseConfigDir().absoluteFilePath(QStringLiteral("trusted_devices")), QSettings::IniFormat);

    const QFile::Permissions strict = QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser | QFile::WriteUser;

    QString keyPath = privateKeyPath();
    QFile privKey(keyPath);
    if (privKey.exists() && privKey.open(QIODevice::ReadOnly)) {

        d->m_privateKey = QSslKey(&privKey, QSsl::Rsa, QSsl::Pem);

    } else {

        d->m_privateKey = Ssl::KeyGenerator::generateRsa(2048);

        if (!privKey.open(QIODevice::ReadWrite | QIODevice::Truncate))  {
            Daemon::instance()->reportError(QStringLiteral("KDE Connect"), tr("Could not store private key file: %1").arg(keyPath));
        } else {
            privKey.setPermissions(strict);
            privKey.write(d->m_privateKey.toPem());
        }
    }

    QString certPath = certificatePath();
    QFile cert(certPath);
    if (cert.exists() && cert.open(QIODevice::ReadOnly)) {

        d->m_certificate = QSslCertificate(&cert, QSsl::Pem);

    } else {

        // No certificate yet. Probably first run. Let's generate one!

        QString uuid = QUuid::createUuid().toString();
        //DbusHelper::filterNonExportableCharacters(uuid);
        qCDebug(coreLogger) << "My id:" << uuid;

        Ssl::CertificateInfo certificateInfo;
        certificateInfo.insert(Ssl::CommonName, uuid);
        certificateInfo.insert(
            Ssl::Organization, QStringLiteral("Richard Liebscher"));

        Ssl::CertificateBuilder certificateBuilder;
        certificateBuilder.setInfo(certificateInfo);
        certificateBuilder.setSerialNumber(10);
        QDateTime startTime = QDateTime::currentDateTime().addYears(-1);
        certificateBuilder.setNotBefore(startTime);
        certificateBuilder.setNotAfter(startTime.addYears(10));

        d->m_certificate = certificateBuilder.selfSigned(d->m_privateKey);

        if (!cert.open(QIODevice::ReadWrite | QIODevice::Truncate))  {
            Daemon::instance()->reportError(QStringLiteral("KDE Connect"), tr("Could not store certificate file: %1").arg(certPath));
        } else {
            cert.setPermissions(strict);
            cert.write(d->m_certificate.toPem());
        }
    }

    //Extra security check
    if (QFile::permissions(keyPath) != strict) {
        qCWarning(coreLogger) << "Warning: KDE Connect private key file has too open permissions " << keyPath;
    }
}

QString KdeConnectConfig::name()
{
    QString defaultName = qgetenv("USER") + '@' + QHostInfo::localHostName();
    QString name = d->m_config->value(QStringLiteral("name"), defaultName).toString();
    return name;
}

void KdeConnectConfig::setName(const QString& name)
{
    d->m_config->setValue(QStringLiteral("name"), name);
    d->m_config->sync();
}

QString KdeConnectConfig::deviceType()
{
    return QStringLiteral("phone"); // TODO
}

QString KdeConnectConfig::deviceId()
{
    return QStringLiteral("<<device-id>>"); // TODO FIXME
}

QString KdeConnectConfig::privateKeyPath()
{
    return baseConfigDir().absoluteFilePath(QStringLiteral("privateKey.pem"));
}

QSslKey KdeConnectConfig::privateKey()
{
    return QSslKey();
}

QSslKey KdeConnectConfig::publicKey()
{
    return d->m_publicKey;
}

QString KdeConnectConfig::certificatePath()
{
    return baseConfigDir().absoluteFilePath(QStringLiteral("certificate.pem"));
}

QSslCertificate KdeConnectConfig::certificate()
{
    return d->m_certificate;
}

QDir KdeConnectConfig::baseConfigDir()
{
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QString kdeconnectConfigPath = QDir(configPath).absoluteFilePath(QStringLiteral("kdeconnect"));
    return QDir(kdeconnectConfigPath);
}

QStringList KdeConnectConfig::trustedDevices()
{
    const QStringList& list = d->m_trustedDevices->childGroups();
    return list;
}


void KdeConnectConfig::addTrustedDevice(const QString& id, const QString& name, const QString& type)
{
    d->m_trustedDevices->beginGroup(id);
    d->m_trustedDevices->setValue(QStringLiteral("name"), name);
    d->m_trustedDevices->setValue(QStringLiteral("type"), type);
    d->m_trustedDevices->endGroup();
    d->m_trustedDevices->sync();

    QDir().mkpath(deviceConfigDir(id).path());
}

KdeConnectConfig::DeviceInfo KdeConnectConfig::getTrustedDevice(const QString& id)
{
    d->m_trustedDevices->beginGroup(id);

    KdeConnectConfig::DeviceInfo info;
    info.deviceName = d->m_trustedDevices->value(QStringLiteral("name"), QLatin1String("unnamed")).toString();
    info.deviceType = d->m_trustedDevices->value(QStringLiteral("type"), QLatin1String("unknown")).toString();

    d->m_trustedDevices->endGroup();
    return info;
}

void KdeConnectConfig::removeTrustedDevice(const QString& deviceId)
{
    d->m_trustedDevices->remove(deviceId);
    d->m_trustedDevices->sync();
    //We do not remove the config files.
}

// Utility functions to set and get a value
void KdeConnectConfig::setDeviceProperty(const QString& deviceId, const QString& key, const QString& value)
{
    d->m_trustedDevices->beginGroup(deviceId);
    d->m_trustedDevices->setValue(key, value);
    d->m_trustedDevices->endGroup();
    d->m_trustedDevices->sync();
}

QString KdeConnectConfig::getDeviceProperty(const QString& deviceId, const QString& key, const QString& defaultValue)
{
    QString value;
    d->m_trustedDevices->beginGroup(deviceId);
    value = d->m_trustedDevices->value(key, defaultValue).toString();
    d->m_trustedDevices->endGroup();
    return value;
}


QDir KdeConnectConfig::deviceConfigDir(const QString& deviceId)
{
    QString deviceConfigPath = baseConfigDir().absoluteFilePath(deviceId);
    return QDir(deviceConfigPath);
}

QDir KdeConnectConfig::pluginConfigDir(const QString& deviceId, const QString& pluginName)
{
    QString deviceConfigPath = baseConfigDir().absoluteFilePath(deviceId);
    QString pluginConfigDir = QDir(deviceConfigPath).absoluteFilePath(pluginName);
    return QDir(pluginConfigDir);
}


