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
    using SailfishConnect::Job::setTitle;
    using SailfishConnect::Job::setErrorString;
    using SailfishConnect::Job::setDescription;
    using SailfishConnect::Job::setTotalBytes;
    using SailfishConnect::Job::setProcessedBytes;
};

#endif // MOCK_JOB_H
