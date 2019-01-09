#include "test.h"
#include "mock_job.h"

using namespace SailfishConnect;
using namespace testing;

class JobTests : public Test {
public:
    JobTests() {
        job.setTitle("test job");
    }

    StrictMock<MockJob> job;
};


TEST_F(JobTests, initState) {
    EXPECT_EQ(job.state(), Job::State::Pending);
    EXPECT_EQ(job.wasCanceled(), false);
}


TEST_F(JobTests, multipleStart) {
    EXPECT_CALL(job, doStart())
            .Times(1);

    job.start();
    job.start();

    EXPECT_EQ(job.state(), Job::State::Running);
    EXPECT_EQ(job.wasCanceled(), false);
}

TEST_F(JobTests, success) {
    EXPECT_CALL(job, doStart()).Times(1);
    EXPECT_CALL(job, onFinished()).Times(1);
    EXPECT_CALL(job, onSuccess()).Times(1);

    job.start();
    job.exit();

    EXPECT_EQ(job.state(), Job::State::Finished);
    EXPECT_EQ(job.wasCanceled(), false);
    EXPECT_EQ(job.errorString(), QString());
}

TEST_F(JobTests, error) {
    EXPECT_CALL(job, doStart()).Times(1);
    EXPECT_CALL(job, onFinished()).Times(1);
    EXPECT_CALL(job, onError()).Times(1);

    job.start();
    job.abort("my fault");

    EXPECT_EQ(job.state(), Job::State::Finished);
    EXPECT_EQ(job.errorString(), "my fault");
    EXPECT_EQ(job.wasCanceled(), false);
}

TEST_F(JobTests, cancel) {
    EXPECT_CALL(job, doStart()).Times(1);
    EXPECT_CALL(job, doCancelling()).WillOnce(Return(true));
    EXPECT_CALL(job, onFinished()).Times(1);

    job.start();
    job.cancel();

    EXPECT_EQ(job.state(), Job::State::Finished);
    EXPECT_EQ(job.errorString(), QString());
    EXPECT_EQ(job.wasCanceled(), true);
}

TEST_F(JobTests, rejectedCancel) {
    EXPECT_CALL(job, doStart()).Times(1);
    EXPECT_CALL(job, doCancelling()).WillOnce(Return(false));

    job.start();
    job.cancel();

    EXPECT_EQ(job.state(), Job::State::Running);
    EXPECT_EQ(job.errorString(), QString());
    EXPECT_EQ(job.wasCanceled(), false);
}

TEST_F(JobTests, cancelFinished) {
    EXPECT_CALL(job, doStart()).Times(1);
    EXPECT_CALL(job, onFinished()).Times(1);
    EXPECT_CALL(job, onSuccess()).Times(1);
    EXPECT_CALL(job, doCancelling()).Times(0);

    job.start();
    job.exit();
    job.cancel();

    EXPECT_EQ(job.state(), Job::State::Finished);
    EXPECT_EQ(job.errorString(), QString());
    EXPECT_EQ(job.wasCanceled(), false);
}

TEST_F(JobTests, cancelPending) {
    EXPECT_CALL(job, onFinished()).Times(1);
    EXPECT_CALL(job, doCancelling()).Times(0);

    job.cancel();

    EXPECT_EQ(job.state(), Job::State::Finished);
    EXPECT_EQ(job.errorString(), QString());
    EXPECT_EQ(job.wasCanceled(), true);
}

TEST_F(JobTests, exitWhileCancelling) {
    EXPECT_CALL(job, doStart()).Times(1);
    EXPECT_CALL(job, onFinished()).Times(1);
    EXPECT_CALL(job, onSuccess()).Times(1);
    EXPECT_CALL(job, doCancelling()).WillOnce(
                InvokeWithoutArgs([this]() {
        job.exit();
        return false;
    }));

    job.start();
    job.cancel();

    EXPECT_EQ(job.state(), Job::State::Finished);
    EXPECT_EQ(job.wasCanceled(), false);
}

TEST_F(JobTests, exitWhileCancelling2) {
    EXPECT_CALL(job, doStart()).Times(1);
    EXPECT_CALL(job, onFinished()).Times(1);
    EXPECT_CALL(job, onSuccess()).Times(1);
    EXPECT_CALL(job, doCancelling()).WillOnce(
                InvokeWithoutArgs([this]() {
        job.exit();
        return true;
    }));

    job.start();
    job.cancel();

    EXPECT_EQ(job.state(), Job::State::Finished);
    EXPECT_EQ(job.wasCanceled(), true);
}
