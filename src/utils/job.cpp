#include "job.h"

namespace SailfishConnect {

Job::Job()
    : QObject(JobManager::instance())
{
    JobManager::instance()->addJob(this);
}

void Job::start()
{
    if (m_isRunning)
        return;

    m_isRunning = true;
    doStart();
}

void Job::setErrorString(const QString& errorString)
{
    if (m_errorString != errorString) {
        m_errorString = errorString;
        emit errorStringChanged();
    }
}

void Job::setDescription(const QString& description)
{
    if (m_description != description) {
        m_description = description;
        emit descriptionChanged();
    }
}

void Job::setTotalBytes(qint64 totalBytes)
{
    if (m_totalBytes != totalBytes) {
        m_totalBytes = totalBytes;
        emit totalBytesChanged();
    }
}

void Job::setProcessedBytes(qint64 processedBytes)
{
    if (m_processedBytes != processedBytes) {
        m_processedBytes = processedBytes;
        emit processedBytesChanged();
    }
}

void Job::exit()
{
    if (!m_isRunning)
        return;

    m_isRunning = false;

    if (m_errorString.isEmpty()) {
        emit success();
    } else {
        emit error();
    }

    emit finished();

    this->deleteLater();
}

void Job::cancel()
{
    if (!m_isRunning)
        return;

    bool canceled = !doCancelling();
    if (canceled || !m_isRunning) {
        m_isRunning = false;
        m_wasCancelled = canceled;
        emit finished();
        this->deleteLater();
    }
}

bool Job::doCancelling()
{
    return false;
}

void Job::setTitle(const QString& title)
{
    if (m_title != title) {
        m_title = title;
        emit titleChanged();
    }
}

JobManager::JobManager()
{

}

JobManager* JobManager::instance()
{
    static JobManager instance;
    return &instance;
}

void JobManager::addJob(Job *job)
{

}

} // namespace SailfishConnect
