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
#include <KLocalizedString>

#include "../helper/cpphelper.h"
#include <compositefiletransferjob.h>
#include <backends/lan/compositeuploadjob.h>
#include <downloadjob.h>


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
    connect(job, SIGNAL(processedAmount(KJob*, KJob::Unit, qulonglong)),
            this, SLOT(onProcessedAmount(KJob*, KJob::Unit, qulonglong)));
    connect(job, SIGNAL(totalAmount(KJob*, KJob::Unit, qulonglong)),
            this, SLOT(onTotalAmount(KJob*, KJob::Unit, qulonglong)));
}

double JobInfo::progress() const
{
    if (!hasProgress())
        return -1.0;

    if (m_totalBytes == 0)
        return 1.0;

    return double(m_processedBytes) / m_totalBytes;
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

    if (title != m_title) {
        m_title = title;
        emit titleChanged();
    }
}

void JobInfo::onTotalAmount(KJob *job, KJob::Unit unit, qulonglong amount)
{
    Q_ASSERT(job == m_impl);

    if (unit == KJob::Bytes) {
        m_totalBytes = amount;
        emit totalBytesChanged();
    } else if (unit == KJob::Files) {
        m_totalFiles = amount;
        emit totalFilesChanged();
    }
}

void JobInfo::onProcessedAmount(KJob *job, KJob::Unit unit, qulonglong amount)
{
    Q_ASSERT(job == m_impl);

    if (unit == KJob::Bytes) {
        m_processedBytes = amount;
        emit processedBytesChanged();
    } else if (unit == KJob::Files) {
        m_processedFiles = amount;
        emit processedFilesChanged();
    }
}

void JobInfo::getTarget()
{
    if (!m_impl) return;

    auto* downloadJob = qobject_cast<DownloadJob*>(m_impl);
    if (downloadJob) {
        QUrl url;
        url.setScheme(QStringLiteral("local"));
        url.setPath(downloadJob->destination());
        setTarget(url);
        m_deviceId = downloadJob->deviceId();
        return;
    }

    auto* uploadJob = qobject_cast<CompositeUploadJob*>(m_impl);
    if (uploadJob) {
        QUrl url;
        url.setScheme(QStringLiteral("remote"));
        url.setPath(QString());
        setTarget(url);
        m_deviceId = uploadJob->deviceId();
        return;
    }
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

    if (m_impl->error() != KJob::NoError) {
        qCDebug(logger)<< "Error" << m_impl->error() <<  m_impl->errorText() <<  m_impl->errorString();
        QString errorString = m_impl->errorText();
        if (errorString.isEmpty()) {
            m_errorString = i18n("Error %1").arg(int(m_impl->error()));
        } else {
            m_errorString = errorString;
        }
    }

    emit stateChanged();
}

JobManager::JobManager(QObject *parent)
    : KJobTrackerInterface(parent)
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
