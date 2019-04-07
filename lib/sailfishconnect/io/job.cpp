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

#include "job.h"

#include <QLoggingCategory>

namespace SailfishConnect {

static Q_LOGGING_CATEGORY(logger, "sailfishconnect.job")

Job::Job(const QString& deviceId, QObject *parent)
    : QObject(parent)
    , m_deviceId(deviceId)
    , m_action(tr("Pending"))
{ }

Job::~Job()
{
    cancel();
}

void Job::start()
{
    if (m_state != State::Pending)
        return;

    m_timer.start();
    qCInfo(logger) << "Job" << m_target.toString() << "is starting";

    m_state = State::Running;
    emit stateChanged();

    doStart();

    if (m_action == tr("Pending")) {
        setAction(tr("Running"));
    }
}

void Job::setErrorString(const QString& errorString)
{
    if (m_errorString != errorString) {
        m_errorString = errorString;
    }
}

void Job::setAction(const QString& action)
{
    if (m_action != action) {
        m_action = action;
        emit actionChanged();
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
    if (processedBytes < m_processedBytes) {
        return; // TODO: Quickfix for broken CopyJob
        qCWarning(logger) << "Back step in processed bytes"
                          << "old:" << m_processedBytes
                          << "new:" << processedBytes;
    }

    if (processedBytes != m_processedBytes) {
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

    qCInfo(logger) << "Job" << m_target.toString() << "was canceled by user";
    bool canceled = isRunning() ? doCancelling() : true;
    if (canceled || !isRunning()) {
        auto old_state = m_state;
        m_state = State::Finished;
        m_canceled = canceled;
        if (old_state != State::Finished) {
            onFinished();
        }
    }
}

bool Job::doCancelling()
{
    return false;
}

void Job::onFinished()
{
    emit finished();
    emit stateChanged();

    if (m_canceled) {
        setAction(tr("Canceled"));
    } else if (!m_errorString.isEmpty()) {
        setAction(tr("Failed"));
    } else {
        setAction(tr("Succeeded"));
    }

    deleteLater();
}

void Job::onSuccess()
{
    qCInfo(logger) << "Job" << m_target.toString() << "was successful in"
                   << m_timer.elapsed() << "ms";
    emit success();
}

void Job::onError()
{
    qCWarning(logger) << "Job" << m_target.toString()
                      << "failed with" << m_errorString;
    emit error();
}

void Job::setTarget(const QUrl& target)
{
    if (m_target != target) {
        m_target = target;
        emit targetChanged();
    }
}

} // namespace SailfishConnect
