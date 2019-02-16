#include "jobmanager.h"

#include <sailfishconnect/io/job.h>
#include <QLoggingCategory>

namespace SailfishConnect {

static Q_LOGGING_CATEGORY(logger, "sailfishconnect.jobmanager")

JobInfo::JobInfo(Job *job, QObject *parent)
    : QObject(parent), m_impl(job), m_deviceId(job->deviceId())
{
    Q_ASSERT(job);

    // only target changes after job finishes
    connect(job, &Job::destroyed, this, &JobInfo::onJobDestroyed);
    connect(job, &Job::targetChanged, this, &JobInfo::onTargetChanged);
    connect(job, &Job::actionChanged, this, &JobInfo::actionChanged);
    connect(job, &Job::totalBytesChanged, this, &JobInfo::totalBytesChanged);
    connect(job, &Job::processedBytesChanged,
            this, &JobInfo::processedBytesChanged);
    connect(job, &Job::stateChanged, this, &JobInfo::onStateChanged);
}

QUrl JobInfo::target() const {
    return m_target;
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
    // WARNING: do not access members of m_impl, because the job is already
    //          destroyed

    m_impl = nullptr;

    if (m_state != Job::State::Finished) {
        qCWarning(logger) << "unfinished job destroyed";
        m_state = Job::State::Finished;
        emit stateChanged();
    }
}

void JobInfo::onStateChanged()
{
    if (m_impl->state() == Job::State::Finished) {
        m_action = m_impl->action();
        m_totalBytes = m_impl->totalBytes();
        m_processedBytes = m_impl->processedBytes();
        m_state = m_impl->state();
        m_canceled = m_impl->canceled();
        m_errorString = m_impl->errorString();
    }

    emit stateChanged();
}

void JobInfo::onTargetChanged()
{
    m_target = m_impl->target();
    emit targetChanged();
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
