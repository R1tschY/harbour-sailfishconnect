#include "job.h"

#include <QLoggingCategory>

namespace SailfishConnect {

static Q_LOGGING_CATEGORY(logger, "sailfishconnect.job")

Job::Job(QObject *parent)
    : QObject(parent)
{ }

Job::~Job()
{
    cancel();
}

void Job::start()
{
    if (m_state != State::Pending)
        return;

    qCInfo(logger) << "Job" << m_title << "is starting";
    m_state = State::Running;
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
    if (isFinished())
        return;

    m_state = State::Finished;

    if (m_errorString.isEmpty()) {
        onSuccess();
    } else {
        onError();
    }

    onFinished();
}

void Job::abort(const QString &error)
{
    if (isFinished())
        return;

    setErrorString(error);
    exit();
}

void Job::cancel()
{
    if (isFinished())
        return;

    qCInfo(logger) << "Job" << m_title << "was canceled by user";
    bool canceled = !doCancelling();
    if (canceled || !isRunning()) {
        m_state = State::Finished;
        m_wasCancelled = canceled;
        onFinished();
    }
}

bool Job::doCancelling()
{
    return false;
}

void Job::onFinished()
{
    emit finished();
}

void Job::onSuccess()
{
    qCInfo(logger) << "Job" << m_title << "was successful";
    emit success();
}

void Job::onError()
{
    qCWarning(logger) << "Job" << m_title << "failed with" << m_errorString;
    emit error();
}

void Job::setTitle(const QString& title)
{
    if (m_title != title) {
        m_title = title;
        emit titleChanged();
    }
}

} // namespace SailfishConnect
