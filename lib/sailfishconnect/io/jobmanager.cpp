#include "jobmanager.h"

#include <sailfishconnect/io/job.h>
#include <QLoggingCategory>

namespace SailfishConnect {

static Q_LOGGING_CATEGORY(logger, "sailfishconnect.jobmanager")

JobInfo::JobInfo(Job *job, QString deviceId, QObject *parent)
    : QObject(parent), m_impl(job), m_deviceId(deviceId)
{
    Q_ASSERT(job);

    connect(job, &Job::destroyed, this, &JobInfo::onJobDestroyed);
    connect(job, &Job::titleChanged, this, &JobInfo::titleChanged);
    connect(job, &Job::descriptionChanged, this, &JobInfo::descriptionChanged);
    connect(job, &Job::totalBytesChanged, this, &JobInfo::totalBytesChanged);
    connect(job, &Job::processedBytesChanged,
            this, &JobInfo::processedBytesChanged);
    connect(job, &Job::stateChanged, this, &JobInfo::stateChanged);
}

QString JobInfo::title() const {
    return m_impl ? m_impl->title() : m_title;
}

QString JobInfo::description() const {
    return m_impl ? m_impl->description() : m_description;
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

bool JobInfo::wasCanceled() const
{
    return m_impl ? m_impl->wasCanceled() : m_wasCanceled;
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
    auto* job = m_impl;
    m_impl = nullptr;

    m_title = job->title();
    m_description = job->description();
    m_totalBytes = job->totalBytes();
    m_processedBytes = job->processedBytes();
    m_state = job->state();
    m_wasCanceled = job->wasCanceled();
    m_errorString = job->errorString();

    if (!m_impl->isFinished()) {
        qCWarning(logger) << "unfinished job destroyed";
        m_state = Job::State::Finished;
        emit stateChanged();
    }
}

JobManager::JobManager(QObject *parent)
{

}

void JobManager::addJob(Job *job, const QString &deviceId)
{
    m_jobs.append(new JobInfo(job, deviceId, this));
    emit jobAdded(m_jobs.back());
}

} // namespace SailfishConnect
