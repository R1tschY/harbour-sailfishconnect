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

#ifndef MOCK_DEVICELINK_H
#define MOCK_DEVICELINK_H

#include <gmock/gmock.h>

#include <sailfishconnect/backend/devicelink.h>
#include <sailfishconnect/networkpacket.h>

class MockDeviceLink : public DeviceLink
{
public:
    MockDeviceLink(const QString& deviceId, LinkProvider* parent);

    MOCK_METHOD0(name, QString());
    MOCK_METHOD2(sendPacket, bool(NetworkPacket&, KJobTrackerInterface*));
    MOCK_METHOD0(userRequestsPair, void());
    MOCK_METHOD0(userRequestsUnpair, void());
    MOCK_METHOD1(setPairStatus, void(PairStatus));
    MOCK_METHOD0(linkShouldBeKeptAlive, bool());
};

#endif // MOCK_DEVICELINK_H
