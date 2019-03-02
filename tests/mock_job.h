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

#ifndef MOCK_JOB_H
#define MOCK_JOB_H

#include <gmock/gmock.h>

#include <sailfishconnect/io/job.h>

class MockJob : public SailfishConnect::Job
{
public:
    MockJob();

    MOCK_METHOD0(doStart, void());
    MOCK_METHOD0(doCancelling, bool());
    MOCK_METHOD0(onFinished, void());
    MOCK_METHOD0(onSuccess, void());
    MOCK_METHOD0(onError, void());

    using SailfishConnect::Job::exit;
    using SailfishConnect::Job::abort;
    using SailfishConnect::Job::setTarget;
    using SailfishConnect::Job::setErrorString;
    using SailfishConnect::Job::setAction;
    using SailfishConnect::Job::setTotalBytes;
    using SailfishConnect::Job::setProcessedBytes;
};

#endif // MOCK_JOB_H
