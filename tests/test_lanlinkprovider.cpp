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
#include <QSslKey>
#include <QSignalSpy>
#include <QVariant>
#include <QCoreApplication>

#include <sailfishconnect/kdeconnectconfig.h>
#include <sailfishconnect/device.h>
#include <sailfishconnect/systeminfo.h>
#include <sailfishconnect/helper/cpphelper.h>
#include <sailfishconnect/networkpacket.h>
#include <sailfishconnect/networkpackettypes.h>
#include <sailfishconnect/backend/lan/lanlinkprovider.h>
#include <sailfishconnect/helper/sslhelper.h>

#include "mock_devicelink.h"
#include "mock_linkprovider.h"
#include "mock_pairinghandler.h"

using namespace SailfishConnect;
using ::testing::Return;
using ::testing::InvokeWithoutArgs;
using ::testing::_;
using ::testing::NiceMock;


class LanLinkProviderTests : public ::testing::Test {
protected:
    LanLinkProviderTests()
        : _argn(0)
        , m_app(_argn, nullptr)
        , kcc(makeUniquePtr<SystemInfo>())
        , m_lanLinkProvider(&kcc, true)
    {
        QStandardPaths::setTestModeEnabled(true);

        deviceId = QStringLiteral("testdevice");
        deviceName = QStringLiteral("Test Device");

        m_lanLinkProvider.onStart();

        m_privateKey = Ssl::KeyGenerator::generateRsa(2048);
        m_certificate = generateCertificate(deviceId, m_privateKey);
        m_identityPacket = QStringLiteral(
                    "{\"id\":1439365924847,\"type\":\"kdeconnect.identity\",\"body\":{\"deviceId\":\"testdevice\",\"deviceName\":\"Test Device\",\"protocolVersion\":6,\"deviceType\":\"phone\",\"tcpPort\":") + QString::number(TEST_PORT) + QStringLiteral("}}\n");

        removeTrustedDevice();

    }

    const int TEST_PORT = 8520;

    int _argn = 0;
    QCoreApplication m_app;

    QString deviceId;
    QString deviceName;
    QString deviceType;
    KdeConnectConfig kcc;
    QString m_identityPacket;
    QSslKey m_privateKey;
    QSslCertificate m_certificate;

    LanLinkProvider m_lanLinkProvider;

    void addTrustedDevice();
    void removeTrustedDevice();
    void setSocketAttributes(QSslSocket* socket);
    void testIdentityPacket(QByteArray& identityPacket);
    QSslCertificate generateCertificate(const QString&, const QSslKey&);
};


//TEST_F(LanLinkProviderTests, pairedDeviceTcpPacketReceived) {
//    addTrustedDevice();

//    QUdpSocket* mUdpServer = new QUdpSocket;
//    bool b = mUdpServer->bind(
//                QHostAddress::LocalHost,
//                udpBroadcastPort,
//                QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
//    ASSERT_TRUE(b);

//    QSignalSpy spy(mUdpServer, SIGNAL(readyRead()));
//    m_lanLinkProvider.onNetworkChange("test");
//    ASSERT_TRUE(!spy.isEmpty() || spy.wait());

//    QByteArray datagram;
//    datagram.resize(mUdpServer->pendingDatagramSize());
//    QHostAddress sender;

//    mUdpServer->readDatagram(datagram.data(), datagram.size(), &sender);

//    testIdentityPacket(datagram);

//    QJsonDocument jsonDocument = QJsonDocument::fromJson(datagram);
//    QJsonObject body = jsonDocument.object().value(QStringLiteral("body")).toObject();
//    int tcpPort = body.value(QStringLiteral("tcpPort")).toInt();

//    QSslSocket socket;
//    QSignalSpy spy2(&socket, SIGNAL(connected()));

//    socket.connectToHost(sender, tcpPort);
//    EXPECT_TRUE(spy2.wait());

//    EXPECT_TRUE(socket.isOpen()); // Socket disconnected immediately

//    socket.write(m_identityPacket.toLatin1());
//    socket.waitForBytesWritten(2000);

//    QSignalSpy spy3(&socket, SIGNAL(encrypted()));

//    setSocketAttributes(&socket);
//    socket.addCaCertificate(kcc.certificate());
//    socket.setPeerVerifyMode(QSslSocket::VerifyPeer);
//    socket.setPeerVerifyName(kcc.name());

//    socket.startServerEncryption();
//    EXPECT_TRUE(spy3.wait());

//    EXPECT_EQ(socket.sslErrors().size(), 0);
//    EXPECT_TRUE(socket.isValid()); // Server socket disconnected
//    EXPECT_TRUE(socket.isEncrypted()); // Server socket not yet encrypted
//    EXPECT_TRUE(!socket.peerCertificate().isNull()); // Peer certificate is null

//    removeTrustedDevice();
//    delete mUdpServer;
//}

void LanLinkProviderTests::testIdentityPacket(QByteArray& identityPacket)
{
    QJsonDocument jsonDocument = QJsonDocument::fromJson(identityPacket);
    QJsonObject jsonObject = jsonDocument.object();
    QJsonObject body = jsonObject.value(QStringLiteral("body")).toObject();

    EXPECT_EQ(jsonObject.value("type").toString(), QString("kdeconnect.identity"));
    EXPECT_TRUE(body.contains("deviceName")); // Device name not found in identity packet
    EXPECT_TRUE(body.contains("deviceId")); // Device id not found in identity packet
    EXPECT_TRUE(body.contains("protocolVersion")); // Protocol version not found in identity packet
    EXPECT_TRUE(body.contains("deviceType")); // Device type not found in identity packet
}

QSslCertificate LanLinkProviderTests::generateCertificate(const QString& commonName, const QSslKey& privateKey)
{
    Ssl::CertificateInfo certificateInfo;
    certificateInfo.insert(Ssl::CommonName, commonName);
    certificateInfo.insert(
        Ssl::Organization, QStringLiteral("Richard Liebscher"));
    certificateInfo.insert(
        Ssl::OrganizationalUnit, QStringLiteral("SailfishConnect"));

    QDateTime startTime = QDateTime::currentDateTime().addYears(-1);

    return Ssl::CertificateBuilder()
            .info(certificateInfo)
            .serialNumber(10)
            .notBefore(startTime)
            .notAfter(startTime.addYears(10))
            .selfSigned(privateKey);
}

void LanLinkProviderTests::setSocketAttributes(QSslSocket* socket)
{
    socket->setPrivateKey(m_privateKey);
    socket->setLocalCertificate(m_certificate);
}

void LanLinkProviderTests::addTrustedDevice()
{
    kcc.addTrustedDevice(deviceId, deviceName, deviceType);
    // Using same certificate from kcc, instead of generating one
    kcc.setDeviceProperty(
                deviceId,
                QStringLiteral("certificate"),
                QString::fromLatin1(kcc.certificate().toPem()));
}

void LanLinkProviderTests::removeTrustedDevice()
{
    kcc.removeTrustedDevice(deviceId);
}
