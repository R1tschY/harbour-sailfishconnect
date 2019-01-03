#ifndef MOCK_DEVICELINK_H
#define MOCK_DEVICELINK_H

#include <gmock/gmock.h>

#include <sailfishconnect/backend/devicelink.h>
#include <sailfishconnect/networkpackage.h>
#include <sailfishconnect/io/jobmanager.h>

class MockDeviceLink : public DeviceLink
{
public:
    MockDeviceLink(const QString& deviceId, LinkProvider* parent);

    MOCK_METHOD0(name, QString());
    MOCK_METHOD2(sendPackage, bool(NetworkPackage&, SailfishConnect::JobManager*));
    MOCK_METHOD0(userRequestsPair, void());
    MOCK_METHOD0(userRequestsUnpair, void());
    MOCK_METHOD1(setPairStatus, void(PairStatus));
    MOCK_METHOD0(linkShouldBeKeptAlive, bool());
};

#endif // MOCK_DEVICELINK_H
