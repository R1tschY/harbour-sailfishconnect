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
    connect(job, &Job::finished, this, &JobInfo::onJobFinished);
    connect(job, &Job::titleChanged, this, &JobInfo::titleChanged);
    connect(job, &Job::descriptionChanged, this, &JobInfo::descriptionChanged);
    connect(job, &Job::totalBytesChanged, this, &JobInfo::totalBytesChanged);
    connect(job, &Job::processedBytesChanged,
            this, &JobInfo::processedBytesChanged);
    connect(job, &Job::stateChanged, this, &JobInfo::stateChanged);
    //connect(job, &Job::errorStringChanged, this, &JobInfo::errorStringChanged);
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

    if (!m_impl->isFinished()) {
        qCWarning(logger) << "unfinished job destroyed";
        onJobFinished();
    }
}

void JobInfo::onJobFinished()
{
    m_state = Job::State::Finished;
//    if (!m_impl->errorString().isEmpty()) {
//        m_description = m_impl->errorString();
//    }

    emit stateChanged();
    emit descriptionChanged();
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
