#include "jobmanager.h"

#include <sailfishconnect/io/job.h>
#include <QLoggingCategory>

namespace SailfishConnect {

static Q_LOGGING_CATEGORY(logger, "sailfishconnect.jobmanager")

JobInfo::JobInfo(Job *job, QObject *parent)
    : QObject(parent), m_impl(job), m_deviceId(job->deviceId())
{
    Q_ASSERT(job);

    m_action = m_impl->action();
    m_state = m_impl->state();

    // only action changes after job finishes
    connect(job, &Job::actionChanged, this, &JobInfo::onActionChanged);
    connect(job, &Job::stateChanged, this, &JobInfo::onStateChanged);
    connect(job, &Job::targetChanged, this, &JobInfo::targetChanged);
    connect(job, &Job::totalBytesChanged, this, &JobInfo::totalBytesChanged);
    connect(job, &Job::processedBytesChanged,
            this, &JobInfo::processedBytesChanged);

    connect(job, &Job::destroyed, this, &JobInfo::onJobDestroyed);
}

QUrl JobInfo::target() const {
    return m_impl ? m_impl->target() : m_target;;
}

QString JobInfo::action() const {
    return m_action;
}

qint64 JobInfo::totalBytes() const {
    return m_impl ? m_impl->totalBytes() : m_totalBytes;
}

qint64 JobInfo::processedBytes() const {
    return m_impl ? m_impl->processedBytes() : m_processedBytes;
}

Job::State JobInfo::state() const {
    return m_state;
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
    m_state = m_impl->state();

    if (m_impl->state() == Job::State::Finished) {
        m_target = m_impl->target();
        m_totalBytes = m_impl->totalBytes();
        m_processedBytes = m_impl->processedBytes();
        m_canceled = m_impl->canceled();
        m_errorString = m_impl->errorString();
    }

    emit stateChanged();
}

void JobInfo::onActionChanged()
{
    m_action = m_impl->action();
    emit actionChanged();
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
