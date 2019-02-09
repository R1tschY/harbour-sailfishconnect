#include "jobmanager.h"

#include <sailfishconnect/io/job.h>
#include <QLoggingCategory>

namespace SailfishConnect {

static Q_LOGGING_CATEGORY(logger, "sailfishconnect.jobmanager")

JobInfo::JobInfo(Job *job, QObject *parent)
    : QObject(parent), m_impl(job), m_deviceId(job->deviceId())
{
    Q_ASSERT(job);

    connect(job, &Job::destroyed, this, &JobInfo::onJobDestroyed);
    connect(job, &Job::targetChanged, this, &JobInfo::targetChanged);
    connect(job, &Job::actionChanged, this, &JobInfo::actionChanged);
    connect(job, &Job::totalBytesChanged, this, &JobInfo::totalBytesChanged);
    connect(job, &Job::processedBytesChanged,
            this, &JobInfo::processedBytesChanged);
    connect(job, &Job::stateChanged, this, &JobInfo::stateChanged);
}

QUrl JobInfo::target() const {
    return m_impl ? m_impl->target() : m_target;
}

QString JobInfo::action() const {
    return m_impl ? m_impl->action() : m_action;
}

qint64 JobInfo::totalBytes() const {
    return m_impl ? m_impl->totalBytes() : m_totalBytes;
}

qint64 JobInfo::processedBytes() const {
    return m_impl ? m_impl->processedBytes() : m_processedBytes;
}

Job::State JobInfo::state() const {
    return m_impl ? m_impl->state() : m_state;
}

bool JobInfo::canceled() const
{
    return m_impl ? m_impl->canceled() : m_canceled;
}

QString JobInfo::errorString() const
{
    return m_impl ? m_impl->errorString() : m_errorString;
}

void JobInfo::cancel()
{
    if (m_impl) {
        m_impl->cancel();
    }
}

void JobInfo::onJobDestroyed()
{
    Q_ASSERT(m_impl);
    auto* job = m_impl;
    m_impl = nullptr;

    m_target = job->target();
    m_action = job->action();
    m_totalBytes = job->totalBytes();
    m_processedBytes = job->processedBytes();
    m_state = job->state();
    m_canceled = job->canceled();
    m_errorString = job->errorString();

    if (!job->isFinished()) {
        qCWarning(logger) << "unfinished job destroyed";
        m_state = Job::State::Finished;
        emit stateChanged();
    }
}

JobManager::JobManager(QObject *parent)
{

}

void JobManager::addJob(Job *job)
{
    m_jobs.append(new JobInfo(job, this));
    emit jobAdded(m_jobs.back());
}

} // namespace SailfishConnect
