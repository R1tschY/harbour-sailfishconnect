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
#include <QSignalSpy>
#include <QVariant>

#include <sailfishconnect/kdeconnectconfig.h>
#include <sailfishconnect/device.h>
#include <sailfishconnect/systeminfo.h>
#include <sailfishconnect/helper/cpphelper.h>
#include <sailfishconnect/networkpacket.h>
#include <sailfishconnect/networkpackettypes.h>

#include "mock_devicelink.h"
#include "mock_linkprovider.h"
#include "mock_pairinghandler.h"

using namespace SailfishConnect;
using ::testing::Return;
using ::testing::InvokeWithoutArgs;
using ::testing::_;
using ::testing::NiceMock;

NetworkPacket createIdentityPacket(
    QString deviceId, QString deviceName, QString deviceType
) {
    NetworkPacket identityPacket(PACKET_TYPE_IDENTITY);
    identityPacket.set(QStringLiteral("deviceId"), deviceId);
    identityPacket.set(QStringLiteral("deviceName"), deviceName);
    identityPacket.set(QStringLiteral("deviceType"), deviceType);
    identityPacket.set(
                QStringLiteral("protocolVersion"),
                NetworkPacket::s_protocolVersion);
    identityPacket.set(
                QStringLiteral("incomingCapabilities"), QStringList());
    identityPacket.set(
                QStringLiteral("outgoingCapabilities"), QStringList());
    return identityPacket;
}


class DeviceTests : public ::testing::Test {
protected:
    DeviceTests()
        : deviceId(QStringLiteral("testdevice")),
          deviceName(QStringLiteral("Test Device")),
          deviceType(QStringLiteral("smartphone")),
          kcc(makeUniquePtr<SystemInfo>()),
          identityPacket(
              createIdentityPacket(deviceId, deviceName, deviceType)),
          link(deviceId, &linkProvider)
    { }

    QString deviceId;
    QString deviceName;
    QString deviceType;
    KdeConnectConfig kcc;
    NetworkPacket identityPacket;
    MockLinkProvider linkProvider;
    MockDeviceLink link;
};

class PairedDeviceTests : public DeviceTests {
protected:
    PairedDeviceTests()
    {
        kcc.addTrustedDevice(deviceId, deviceName, deviceType);
        // Using same certificate from kcc, instead of generating one
        kcc.setDeviceProperty(
                    deviceId,
                    QStringLiteral("certificate"),
                    QString::fromLatin1(kcc.certificate().toPem()));
    }
};

class UnpairedDeviceTests : public DeviceTests {
protected:
    UnpairedDeviceTests()
    {
        kcc.removeTrustedDevice(deviceId);
    }
};



TEST_F(PairedDeviceTests, addRemoveLinkToPairedDevice) {
    Device device(nullptr, &kcc, deviceId);

    EXPECT_EQ(device.id(), deviceId);
    EXPECT_EQ(device.name(), deviceName);
    EXPECT_EQ(device.type(), deviceType);

    EXPECT_EQ(device.isTrusted(), true);
    EXPECT_EQ(device.isReachable(), false);

    // Add link
    device.addLink(identityPacket, &link);

    EXPECT_EQ(device.isReachable(), true);
    EXPECT_EQ(device.availableLinks().contains(linkProvider.name()), true);

    // Remove link
    device.removeLink(&link);

    EXPECT_EQ(device.isReachable(), false);
    EXPECT_EQ(device.availableLinks().contains(linkProvider.name()), false);

    device.unpair();
    EXPECT_EQ(device.isTrusted(), false);

    device.removeLink(&link);
}

TEST_F(UnpairedDeviceTests, addRemoveLinkToUnpairedDevice)
{
    // add link through ctor
    Device device(nullptr, &kcc, identityPacket, &link);

    EXPECT_EQ(device.id(), deviceId);
    EXPECT_EQ(device.name(), deviceName);
    EXPECT_EQ(device.type(), deviceType);

    EXPECT_EQ(device.isTrusted(), false);
    EXPECT_EQ(device.isReachable(), true);
    EXPECT_EQ(device.availableLinks().contains(linkProvider.name()), true);

    // Remove link
    device.removeLink(&link);

    EXPECT_EQ(device.isTrusted(), false);
    EXPECT_EQ(device.isReachable(), false);
    EXPECT_EQ(device.availableLinks().contains(linkProvider.name()), false);

    // add link
    device.addLink(identityPacket, &link);

    EXPECT_EQ(device.isTrusted(), false);
    EXPECT_EQ(device.isReachable(), true);
    EXPECT_EQ(device.availableLinks().contains(linkProvider.name()), true);

    device.removeLink(&link);
}

TEST_F(DeviceTests, changeNameTypePaired1)
{
    kcc.addTrustedDevice(deviceId, "old name", "desktop");
    // Using same certificate from kcc, instead of generating one
    kcc.setDeviceProperty(
                deviceId,
                QStringLiteral("certificate"),
                QString::fromLatin1(kcc.certificate().toPem()));

    {
        Device device(nullptr, &kcc, deviceId);

        EXPECT_EQ(device.name(), "old name");
        EXPECT_EQ(device.type(), "desktop");

        EXPECT_EQ(device.isTrusted(), true);
        EXPECT_EQ(device.isReachable(), false);

        // Add link
        auto identityPacket1 = createIdentityPacket(
                    deviceId, "new name", "laptop");
        device.addLink(identityPacket1, &link);

        EXPECT_EQ(device.name(), "new name");
        EXPECT_EQ(device.type(), "laptop");

        EXPECT_EQ(device.isReachable(), true);
        EXPECT_EQ(device.isTrusted(), true);

        device.removeLink(&link);
    }

    // remove device to check that name and type are persisted
    {
        Device device(nullptr, &kcc, deviceId);

        EXPECT_EQ(device.name(), "new name");
        EXPECT_EQ(device.type(), "laptop");

        EXPECT_EQ(device.isTrusted(), true);
        EXPECT_EQ(device.isReachable(), false);
    }
}

TEST_F(PairedDeviceTests, changeNameTypePaired2)
{
    {
        Device device(nullptr, &kcc, deviceId, "new name", "laptop");

        EXPECT_EQ(device.name(), "new name");
        EXPECT_EQ(device.type(), "laptop");

        EXPECT_EQ(device.isReachable(), false);
        EXPECT_EQ(device.isTrusted(), true);
    }

    // remove device to check that name and type are persisted
    {
        Device device(nullptr, &kcc, deviceId);

        EXPECT_EQ(device.name(), "new name");
        EXPECT_EQ(device.type(), "laptop");

        EXPECT_EQ(device.isTrusted(), true);
        EXPECT_EQ(device.isReachable(), false);
    }
}

TEST_F(UnpairedDeviceTests, changeNameTypeUnpaired)
{
    // add link through ctor
    auto identityPacket1 = createIdentityPacket(
                deviceId, "old name", "desktop");
    Device device(nullptr, &kcc, identityPacket1, &link);

    EXPECT_EQ(device.name(), "old name");
    EXPECT_EQ(device.type(), "desktop");
    EXPECT_EQ(device.isTrusted(), false);
    EXPECT_EQ(device.isReachable(), true);

    device.removeLink(&link);
    auto identityPacket2 = createIdentityPacket(
                deviceId, "new name", "laptop");
    device.addLink(identityPacket2, &link);

    EXPECT_EQ(device.name(), "new name");
    EXPECT_EQ(device.type(), "laptop");
    EXPECT_EQ(device.isTrusted(), false);
    EXPECT_EQ(device.isReachable(), true);

    device.removeLink(&link);
}

TEST_F(PairedDeviceTests, unpair)
{
    auto identityPacket = createIdentityPacket(
                deviceId, deviceName, deviceType);
    Device device(nullptr, &kcc, identityPacket, &link);

    EXPECT_EQ(device.isTrusted(), true);
    EXPECT_EQ(device.isReachable(), true);

    EXPECT_CALL(link, userRequestsUnpair()).Times(1);
    QSignalSpy spy(&device, &Device::trustedChanged);

    device.unpair();

    EXPECT_EQ(spy, toVVList({{false}}));
    EXPECT_EQ(device.isTrusted(), false);
    EXPECT_EQ(device.isReachable(), true);

    device.removeLink(&link);
}

TEST_F(UnpairedDeviceTests, pair)
{
    auto identityPacket = createIdentityPacket(
                deviceId, deviceName, deviceType);
    Device device(nullptr, &kcc, identityPacket, &link);

    // request pair (on local side)

    EXPECT_CALL(link, userRequestsPair()).Times(1);

    device.requestPair();

    EXPECT_EQ(device.isTrusted(), false);
    EXPECT_EQ(device.isReachable(), true);

    // accept pair (on remote side)

    QSignalSpy spy(&device, &Device::trustedChanged);
    link.pairStatusChanged(DeviceLink::Paired);

    EXPECT_EQ(spy, toVVList({{true}}));
    EXPECT_EQ(device.isTrusted(), true);
    EXPECT_EQ(device.isReachable(), true);

    device.removeLink(&link);
}

TEST_F(UnpairedDeviceTests, acceptPair)
{
    auto identityPacket = createIdentityPacket(
                deviceId, deviceName, deviceType);
    Device device(nullptr, &kcc, identityPacket, &link);
    QSignalSpy spy_pairing(&device, &Device::hasPairingRequestsChanged);

    // request pair (on remote side)

    auto* pairingHandler = new MockPairingHandler(&link);
    emit link.pairingRequest(pairingHandler);
    emit link.pairStatusChanged(DeviceLink::NotPaired);

    EXPECT_EQ(device.isTrusted(), false);
    EXPECT_EQ(device.isReachable(), true);
    EXPECT_EQ(device.hasPairingRequests(), true);
    EXPECT_EQ(spy_pairing, toVVList({{true}}));
    spy_pairing.clear();

    // accept pair (on local side)

    QSignalSpy spy_trusted(&device, &Device::trustedChanged);
    EXPECT_CALL(*pairingHandler, acceptPairing()).WillOnce(
                InvokeWithoutArgs([this, pairingHandler](){
        emit link.pairingRequestExpired(pairingHandler);
        emit link.pairStatusChanged(DeviceLink::Paired);
        return true;
    }));
    device.acceptPairing();

    EXPECT_EQ(spy_trusted, toVVList({{true}}));
    EXPECT_EQ(device.isTrusted(), true);
    EXPECT_EQ(device.isReachable(), true);
    EXPECT_EQ(device.hasPairingRequests(), false);
    EXPECT_EQ(spy_pairing, toVVList({{false}}));

    device.removeLink(&link);
}

TEST_F(UnpairedDeviceTests, rejectPair)
{
    auto identityPacket = createIdentityPacket(
                deviceId, deviceName, deviceType);
    Device device(nullptr, &kcc, identityPacket, &link);
    QSignalSpy spy_pairing(&device, &Device::hasPairingRequestsChanged);

    // request pair (on remote side)

    auto* pairingHandler = new MockPairingHandler(&link);
    emit link.pairingRequest(pairingHandler);
    emit link.pairStatusChanged(DeviceLink::NotPaired);

    EXPECT_EQ(device.isTrusted(), false);
    EXPECT_EQ(device.isReachable(), true);
    EXPECT_EQ(device.hasPairingRequests(), true);
    EXPECT_EQ(spy_pairing, toVVList({{true}}));
    spy_pairing.clear();

    // reject pair (on local side)

    QSignalSpy spy_trusted(&device, &Device::trustedChanged);
    EXPECT_CALL(*pairingHandler, acceptPairing()).WillOnce(
                InvokeWithoutArgs([this, pairingHandler](){
        emit link.pairingRequestExpired(pairingHandler);
        emit link.pairStatusChanged(DeviceLink::NotPaired);
        return true;
    }));
    device.acceptPairing();

    EXPECT_EQ(spy_trusted, toVVList({{false}}));
    EXPECT_EQ(device.isTrusted(), false);
    EXPECT_EQ(device.isReachable(), true);
    EXPECT_EQ(device.hasPairingRequests(), false);
    EXPECT_EQ(spy_pairing, toVVList({{false}}));

    device.removeLink(&link);
}

TEST_F(UnpairedDeviceTests, rejectedPair)
{
    auto identityPacket = createIdentityPacket(
                deviceId, deviceName, deviceType);
    Device device(nullptr, &kcc, identityPacket, &link);

    // request pair (on local side)

    EXPECT_CALL(link, userRequestsPair()).Times(1);

    device.requestPair();

    EXPECT_EQ(device.isTrusted(), false);
    EXPECT_EQ(device.isReachable(), true);

    // reject pair (on remote side)

    QSignalSpy spy(&device, &Device::trustedChanged);
    emit link.pairStatusChanged(DeviceLink::NotPaired);

    EXPECT_EQ(spy, toVVList({{false}}));
    EXPECT_EQ(device.isTrusted(), false);
    EXPECT_EQ(device.isReachable(), true);

    device.removeLink(&link);
}
