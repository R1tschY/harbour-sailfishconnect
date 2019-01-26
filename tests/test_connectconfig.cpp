/*
 * Copyright 2015 Vineet Garg <grgvineet@gmail.com>
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

#include "test.h"

#include <QSslCertificate>

#include <sailfishconnect/kdeconnectconfig.h>
#include <sailfishconnect/systeminfo.h>
#include <sailfishconnect/helper/cpphelper.h>

using namespace SailfishConnect;

class ConnectConfigTests : public ::testing::Test {
protected:
    ConnectConfigTests()
        : config(makeUniquePtr<SystemInfo>())
    { }

    static void SetUpTestCase() {
        QStandardPaths::setTestModeEnabled(true);

        auto configDir = QDir(QStandardPaths::writableLocation(
                    QStandardPaths::AppConfigLocation));
        EXPECT_TRUE(configDir.removeRecursively());
    }

    KdeConnectConfig config;
};


TEST_F(ConnectConfigTests, addTrustedDevice) {
    config.addTrustedDevice(
                QStringLiteral("testdevice"),
                QStringLiteral("Test Device"),
                QStringLiteral("phone"));
    KdeConnectConfig::DeviceInfo devInfo =
            config.getTrustedDevice(QStringLiteral("testdevice"));
    EXPECT_EQ(devInfo.deviceName, QString("Test Device"));
    EXPECT_EQ(devInfo.deviceType, QString("phone"));
}

TEST_F(ConnectConfigTests, generateCertificate) {
    QSslCertificate certificate = config.certificate();
    EXPECT_EQ(certificate.serialNumber().toInt(0, 16), 10);
    EXPECT_EQ(certificate.subjectInfo(
                  QSslCertificate::SubjectInfo::CommonName).constFirst(), config.deviceId());
    EXPECT_EQ(certificate.subjectInfo(
                  QSslCertificate::SubjectInfo::Organization).constFirst(), QString("Richard Liebscher"));
    EXPECT_EQ(certificate.subjectInfo(
                  QSslCertificate::OrganizationalUnitName).constFirst(), QString("SailfishConnect"));
}

TEST_F(ConnectConfigTests, removeTrustedDevice) {
    config.removeTrustedDevice(QStringLiteral("testdevice"));
    KdeConnectConfig::DeviceInfo devInfo = config.getTrustedDevice(QStringLiteral("testdevice"));
    EXPECT_EQ(devInfo.deviceName, QString("unnamed"));
    EXPECT_EQ(devInfo.deviceType, QString("unknown"));
}
