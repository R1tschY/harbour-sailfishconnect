#ifndef MOCK_PAIRINGHANDLER_H
#define MOCK_PAIRINGHANDLER_H

#include <gmock/gmock.h>
#include <sailfishconnect/backend/pairinghandler.h>


class MockPairingHandler : public PairingHandler
{
public:
    MockPairingHandler(DeviceLink* parent);

    MOCK_METHOD1(packetReceived, void(const NetworkPacket&));
    MOCK_METHOD0(unpair, void());

public:
    MOCK_METHOD0(requestPairing, bool());
    MOCK_METHOD0(acceptPairing, bool());
    MOCK_METHOD0(rejectPairing, void());
};

#endif // MOCK_PAIRINGHANDLER_H
