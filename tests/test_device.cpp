/*
 * Copyright 2015 Vineet Garg <grgvineet@gmail.com>
 * Copyright 2018 Richard Liebscher <richard.liebscher@gmail.com>
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

#include "test.h"
#include <gmock/gmock.h>

#include <QSslCertificate>

#include <sailfishconnect/kdeconnectconfig.h>
#include <sailfishconnect/device.h>
#include <sailfishconnect/systeminfo.h>
#include <sailfishconnect/helper/cpphelper.h>
#include <sailfishconnect/networkpackage.h>
#include <sailfishconnect/networkpackagetypes.h>

#include "mock_devicelink.h"
#include "mock_linkprovider.h"

using namespace SailfishConnect;

class DeviceTests : public ::testing::Test {
protected:
    DeviceTests()
        : deviceId(QStringLiteral("testdevice")),
          deviceName(QStringLiteral("Test Device")),
          deviceType(QStringLiteral("smartphone")),
          kcc(makeUniquePtr<SystemInfo>()),
          identityPackage(PACKAGE_TYPE_IDENTITY)
    {
        identityPackage.set(QStringLiteral("deviceId"), deviceId);
        identityPackage.set(QStringLiteral("deviceName"), deviceName);
        identityPackage.set(QStringLiteral("deviceType"), deviceType);
        identityPackage.set(
                    QStringLiteral("protocolVersion"),
                    NetworkPackage::s_protocolVersion);
        identityPackage.set(
                    QStringLiteral("incomingCapabilities"), QStringList());
        identityPackage.set(
                    QStringLiteral("outgoingCapabilities"), QStringList());
    }

    QString deviceId;
    QString deviceName;
    QString deviceType;
    KdeConnectConfig kcc;
    NetworkPackage identityPackage;
};

TEST_F(DeviceTests, addTrustedDevice) {
    kcc.addTrustedDevice(deviceId, deviceName, deviceType);
    // Using same certificate from kcc, instead of generating one
    kcc.setDeviceProperty(
                deviceId,
                QStringLiteral("certificate"),
                QString::fromLatin1(kcc.certificate().toPem()));

    Device device(nullptr, &kcc, deviceId, deviceName, deviceType);

    EXPECT_EQ(device.id(), deviceId);
    EXPECT_EQ(device.name(), deviceName);
    EXPECT_EQ(device.type(), deviceType);

    EXPECT_EQ(device.isTrusted(), true);
    EXPECT_EQ(device.isReachable(), false);

    // Add link
    MockLinkProvider linkProvider;
    MockDeviceLink link(deviceId, &linkProvider);
    device.addLink(identityPackage, &link);

    EXPECT_EQ(device.isReachable(), true);
    EXPECT_EQ(device.availableLinks().contains(linkProvider.name()), true);

    // Remove link
    device.removeLink(&link);

    EXPECT_EQ(device.isReachable(), false);
    EXPECT_EQ(device.availableLinks().contains(linkProvider.name()), false);

    device.unpair();
    EXPECT_EQ(device.isTrusted(), false);
}
