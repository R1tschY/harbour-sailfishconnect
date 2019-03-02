/*
 * Copyright 2019 Richard Liebscher <richard.liebscher@gmail.com>.
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
