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
#include "../utils/sslhelper.h"
#include "systeminfo.h"
#include "daemon.h"

using namespace SailfishConnect;

struct KdeConnectConfigPrivate {
    QDir m_configBaseDir;

    QSslKey m_privateKey;
    QSslCertificate m_certificate;
    QString m_deviceId;
    QString m_name;

    QSettings* m_config;
    QSettings* m_trustedDevices;

    std::unique_ptr<SailfishConnect::SystemInfo> systemInfo;
};

static const QFile::Permissions strictFilePermissions =
        QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser
        | QFile::WriteUser;

KdeConnectConfig* KdeConnectConfig::instance()
{
    return Daemon::instance()->config();
}

KdeConnectConfig::KdeConnectConfig(std::unique_ptr<SailfishConnect::SystemInfo> systemInfo)
    : d(new KdeConnectConfigPrivate)
{
    d->systemInfo = std::move(systemInfo);

    createBaseConfigDir();
    d->m_config = new QSettings(configPath(), QSettings::IniFormat);
    d->m_trustedDevices = new QSettings(
                trustedDevicesConfigPath(),
                QSettings::IniFormat);

    createName();
    createCertificate();
}

void KdeConnectConfig::createBaseConfigDir()
{
    QString configPath = QStandardPaths::writableLocation(
                QStandardPaths::ConfigLocation);
    QString kdeconnectConfigPath =
            QDir(configPath).absoluteFilePath(QStringLiteral("kdeconnect"));
    d->m_configBaseDir = QDir(kdeconnectConfigPath);

    // Make sure base directory exists
    QDir().mkpath(d->m_configBaseDir.absolutePath());
}

void KdeConnectConfig::createCertificate()
{
    QString keyPath = privateKeyPath();
    QFile keyFile(keyPath);
    if (keyFile.exists() && keyFile.open(QIODevice::ReadOnly)) {
        qCInfo(coreLogger) << "using exitsing RSA key at" << keyPath;
        d->m_privateKey = QSslKey(&keyFile, QSsl::Rsa, QSsl::Pem);
        if (d->m_privateKey.isNull()) {
            qCCritical(coreLogger) << "reading private key failed";
        }
        keyFile.close();
    }
    if (d->m_privateKey.isNull()) {
        qCInfo(coreLogger) << "generate private RSA key to" << keyPath;

        d->m_privateKey = Ssl::KeyGenerator::generateRsa(2048);
        if (d->m_privateKey.isNull()) {
            qCCritical(coreLogger) << "generating private key failed";
            return;
        }

        if (!keyFile.open(QIODevice::ReadWrite | QIODevice::Truncate))  {
            qCCritical(coreLogger)
                    << "Could not store private key file:" << keyPath;
        } else {
            keyFile.setPermissions(strictFilePermissions);
            keyFile.write(d->m_privateKey.toPem());
            keyFile.close();
        }
    }

    QString certPath = certificatePath();
    QFile certFile(certPath);
    if (certFile.exists() && certFile.open(QIODevice::ReadOnly)) {
        d->m_certificate = QSslCertificate(&certFile, QSsl::Pem);
        if (d->m_certificate.isNull()) {
            qCCritical(coreLogger) << "reading certificate failed";
        }
        createDeviceId();
        certFile.close();
    }
    if (d->m_certificate.isNull()) {
        // No certificate yet. Probably first run. Let's generate one!

        qCInfo(coreLogger) << "generate certificate to" << certPath;

        createDeviceId();

        Ssl::CertificateInfo certificateInfo;
        certificateInfo.insert(Ssl::CommonName, d->m_deviceId);
        certificateInfo.insert(
            Ssl::Organization, QStringLiteral("Richard Liebscher")); // TODO

        Ssl::CertificateBuilder certificateBuilder;
        certificateBuilder.setInfo(certificateInfo);
        certificateBuilder.setSerialNumber(10);
        QDateTime startTime = QDateTime::currentDateTime().addYears(-1);
        certificateBuilder.setNotBefore(startTime);
        certificateBuilder.setNotAfter(startTime.addYears(10));

        d->m_certificate = certificateBuilder.selfSigned(d->m_privateKey);
        if (d->m_certificate.isNull()) {
            qCCritical(coreLogger) << "generating certificate failed";
            return;
        }

        if (!certFile.open(QIODevice::ReadWrite | QIODevice::Truncate))  {
            qCCritical(coreLogger)
                    << "Could not store certificate file:" << certPath;
        } else {
            certFile.setPermissions(strictFilePermissions);
            certFile.write(d->m_certificate.toPem());
            certFile.close();
        }
    }

    // Extra security check
    if (QFile::permissions(keyPath) != strictFilePermissions) {
        qCWarning(coreLogger)
                << "Warning: KDE Connect private key file has too open permissions"
                << keyPath;
    }
}

void KdeConnectConfig::createDeviceId()
{
    if (!d->m_certificate.isNull()) {
        d->m_deviceId =
                d->m_certificate.subjectInfo(QSslCertificate::CommonName).at(0);
    } else {
        d->m_deviceId = QUuid::createUuid().toString();
    }
    qCDebug(coreLogger) << "My id:" << d->m_deviceId;
}

void KdeConnectConfig::createName()
{
    QString name = d->m_config->value(QStringLiteral("name")).toString();
    if (name.isEmpty()) {
        name = d->systemInfo->defaultName();
    }
    d->m_name = name;
}

QString KdeConnectConfig::name() const
{
    return d->m_name;
}

void KdeConnectConfig::setName(const QString& name)
{
    d->m_config->setValue(QStringLiteral("name"), name);
    d->m_config->sync();

    d->m_name = name;
}

bool KdeConnectConfig::valid() const
{
    return !d->m_certificate.isNull() && !d->m_privateKey.isNull();
}

QString KdeConnectConfig::deviceType() const
{
    return d->systemInfo->deviceType();
}

QString KdeConnectConfig::deviceId() const
{
    return d->m_deviceId;
}

QString KdeConnectConfig::privateKeyPath() const
{
    return baseConfigDir().absoluteFilePath(QStringLiteral("privateKey.pem"));
}

QSslKey KdeConnectConfig::privateKey() const
{
    return d->m_privateKey;
}

QString KdeConnectConfig::certificatePath() const
{
    return baseConfigDir().absoluteFilePath(QStringLiteral("certificate.pem"));
}

QSslCertificate KdeConnectConfig::certificate() const
{
    return d->m_certificate;
}

QString KdeConnectConfig::configPath() const
{
    return baseConfigDir().absoluteFilePath(QStringLiteral("config"));
}

QString KdeConnectConfig::trustedDevicesConfigPath() const
{
    return baseConfigDir().absoluteFilePath(QStringLiteral("trusted_devices"));
}

QDir KdeConnectConfig::baseConfigDir() const
{
    return d->m_configBaseDir;
}

QStringList KdeConnectConfig::trustedDevices() const
{
    return d->m_trustedDevices->childGroups();
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

KdeConnectConfig::DeviceInfo KdeConnectConfig::getTrustedDevice(const QString& id) const
{
    d->m_trustedDevices->beginGroup(id);

    KdeConnectConfig::DeviceInfo info;
    info.deviceName = d->m_trustedDevices->value(
                QStringLiteral("name"), QLatin1String("unnamed")).toString();
    info.deviceType = d->m_trustedDevices->value(
                QStringLiteral("type"), QLatin1String("unknown")).toString();

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
void KdeConnectConfig::setDeviceProperty(
        const QString& deviceId, const QString& key, const QString& value)
{
    d->m_trustedDevices->beginGroup(deviceId);
    d->m_trustedDevices->setValue(key, value);
    d->m_trustedDevices->endGroup();
    d->m_trustedDevices->sync();
}

QString KdeConnectConfig::getDeviceProperty(
        const QString& deviceId, const QString& key, const QString& defaultValue) const
{
    QString value;
    d->m_trustedDevices->beginGroup(deviceId);
    value = d->m_trustedDevices->value(key, defaultValue).toString();
    d->m_trustedDevices->endGroup();
    return value;
}


QDir KdeConnectConfig::deviceConfigDir(const QString& deviceId) const
{
    QString deviceConfigPath = baseConfigDir().absoluteFilePath(deviceId);
    return QDir(deviceConfigPath);
}

QDir KdeConnectConfig::pluginConfigDir(
        const QString& deviceId, const QString& pluginName) const
{
    return QDir(baseConfigDir().absoluteFilePath(deviceId + "/" + pluginName));
}


