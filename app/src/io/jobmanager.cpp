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

#include "jobmanager.h"

#include <QLoggingCategory>

#include "../helper/cpphelper.h"
#include <compositefiletransferjob.h>
#include <backends/lan/compositeuploadjob.h>


namespace SailfishConnect {

static Q_LOGGING_CATEGORY(logger, "sailfishconnect.jobmanager")

JobInfo::JobInfo(KJob *job, QObject *parent)
    : QObject(parent), m_impl(job)
{
    Q_ASSERT(job);

    m_state = QStringLiteral("running");

    // TODO: use interface
    // auto* copyJob = qobject_cast<CopyJob*>(m_impl);
    // if (copyJob) {
    //     m_deviceId = copyJob->deviceId();
    // }

    getTarget();

    connect(job, &KJob::description, this, &JobInfo::onDescription);
    connect(job, &KJob::finished, this, &JobInfo::onFinished);
    connect(job, &KJob::result, this, &JobInfo::onResult);
    connect(job, SIGNAL(processedAmount(KJob*, KJob::Unit, qulonglong)),
            this, SLOT(onProcessedAmount(KJob*, KJob::Unit, qulonglong)));
    connect(job, SIGNAL(totalAmount(KJob*, KJob::Unit, qulonglong)),
            this, SLOT(onTotalAmount(KJob*, KJob::Unit, qulonglong)));
}

void JobInfo::cancel()
{
    if (m_impl) {
        m_impl->kill();
    }
}

void JobInfo::onDescription(
        KJob *job,
        const QString &title,
        const QPair<QString, QString> &field1,
        const QPair<QString, QString> &field2)
{
    Q_ASSERT(job == m_impl);

    m_title = title;
    m_field1 = field1;
    m_field2 = field2;
    getTarget();
}

void JobInfo::onTotalAmount(KJob *job, KJob::Unit unit, qulonglong amount)
{
    Q_ASSERT(job == m_impl);

    if (unit == KJob::Bytes) {
        m_totalBytes = amount;
        emit totalBytesChanged();
    }
}

void JobInfo::onProcessedAmount(KJob *job, KJob::Unit unit, qulonglong amount)
{
    Q_ASSERT(job == m_impl);

    if (unit == KJob::Bytes) {
        m_processedBytes = amount;
        emit processedBytesChanged();
    }
}

void JobInfo::getTarget()
{
    if (!m_impl) return;

    // TODO: use interface

    // auto* downloadJob = qobject_cast<CompositeFileTransferJob*>(m_impl);
    // if (downloadJob) {
    //     QUrl url;
    //     url.setScheme("local");
    //     url.setPath(downloadJob->destination().toLocalFile());
    //     setTarget(url);
    //     return;
    // }

    // auto* uploadJob = qobject_cast<CompositeUploadJob*>(m_impl);
    // if (uploadJob) {
    //     QUrl url;
    //     url.setScheme("remote");
    //     url.setPath(uploadJob->fileName());
    //     setTarget(url);
    //     return;
    // }
}

void JobInfo::setTarget(const QUrl& value)
{
    if (value != m_target) {
        m_target = value;
        emit targetChanged();
    }
}

void JobInfo::onFinished()
{
    if (m_impl->error() == KJob::KilledJobError) {
        m_state = QStringLiteral("canceled");
    } else {
        m_state = QStringLiteral("finished");
    }

    emit stateChanged();
}

void JobInfo::onResult()
{
    if (m_impl->error()) {
        m_errorString = m_impl->errorText();
    }
}

JobManager::JobManager(QObject *parent)
{

}

void JobManager::unregisterJob(KJob *job)
{
    QList<JobInfo*> toRemove;

    for (auto& ji : asConst(m_jobs)) {
        if (ji->job() == job) {
            toRemove.append(ji);
        }
    }

    for (auto& ji : asConst(toRemove)) {
        m_jobs.removeOne(ji);
        emit jobRemoved(ji);
    }
}

void JobManager::registerJob(KJob *job)
{
    m_jobs.append(new JobInfo(job, this));
    emit jobAdded(m_jobs.back());
}

} // namespace SailfishConnect
