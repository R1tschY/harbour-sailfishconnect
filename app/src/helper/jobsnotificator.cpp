/*
 * Copyright 2020 Richard Liebscher <richard.liebscher@gmail.com>.
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

#include "jobsnotificator.h"

#include <QGuiApplication>
#include <KJobTrackerInterface>
#include <QLoggingCategory>
#include <KLocalizedString>

#include <core/daemon.h>
#include <core/device.h>
#include <notification.h>
#include "../ui.h"


static Q_LOGGING_CATEGORY(logger, "SailfishConnect.JobsNotificator")

namespace SailfishConnect {

JobsNotificator::JobsNotificator(JobManager *jobManager)
    : m_jobManager(jobManager)
{
    connect(m_jobManager, &JobManager::jobAdded,
            this, &JobsNotificator::addJob);

    const auto jobs = m_jobManager->jobs();
    for (auto* job : jobs) {
        addJob(job);
    }
}

void JobsNotificator::connectJob(JobInfo *job)
{
    auto callback = [=](){ jobChanged(job); };

    connect(job, &JobInfo::processedBytesChanged, this, callback);
    connect(job, &JobInfo::totalBytesChanged, this, callback);
    connect(job, &JobInfo::processedFilesChanged, this, callback);
    connect(job, &JobInfo::totalFilesChanged, this, callback);
    connect(job, &JobInfo::stateChanged, this, callback);
    connect(job, &JobInfo::titleChanged, this, callback);
}

void JobsNotificator::jobChanged(JobInfo *job)
{
    bool isUpload = job->target().scheme() == QStringLiteral("remote");

    Notification* notification = m_jobs.value(job);
    Q_ASSERT(notification != nullptr);

    if (job->state() == QStringLiteral("running")) {
        notification->setHintValue("x-nemo-progress", job->progress());

        if (job->totalFiles() > 1) {
            notification->setBody(job->title());
            notification->setItemCount(job->totalFiles());
        }
    } else {
        quint32 id = notification->replacesId();
        notification->deleteLater();

        // recreate to get rid of the progress bar
        Device* device = Daemon::instance()->getDevice(job->deviceId());
        QString fileName = job->target().fileName();

        notification = new Notification(this);
        m_jobs[job] = notification;
        notification->setBody(fileName);
        notification->setReplacesId(id);

        QString body;
        if (job->totalFiles() > 1) {
            body = job->title();
            notification->setItemCount(job->totalFiles());
        } else {
            body = device ? i18n("%1 from %2").arg(fileName, device->name()) : fileName;
        }

        if (job->state() == QStringLiteral("finished")) {
            if (job->errorString().isEmpty()) {
                if (isUpload) {
                    notification->setSummary(i18n("Upload succedded"));
                } else {
                    notification->setSummary(i18n("Download succedded"));
                }
            } else {
                if (isUpload) {
                    notification->setSummary(i18n("Upload failed"));
                } else {
                    notification->setSummary(i18n("Download failed"));
                }
                body = job->errorString();
            }

            notification->setPreviewBody(body);
            notification->setPreviewSummary(notification->summary());
        } else if (job->state() == QStringLiteral("canceled")) {
            if (isUpload) {
                notification->setSummary(i18n("Upload aborted"));
            } else {
                notification->setSummary(i18n("Download aborted"));
            }
        } else {
            return;
        }

        notification->setBody(body);
    }
    notification->publish();
}

void JobsNotificator::addJob(JobInfo *job)
{
    bool isUpload = job->target().scheme() == QStringLiteral("remote");
    QString fileName = job->target().fileName();
    Device* device = Daemon::instance()->getDevice(job->deviceId());
    QString body = device ? i18n("%1 from %2").arg(fileName, device->name()) : fileName;

    Notification* notification = new Notification(this);
    notification->setBody(body);
    notification->setItemCount(job->processedFiles());
    if (isUpload) {
        notification->setSummary(i18n("Uploading ..."));
    } else {
        QString summary = i18n("Downloading ...");
        notification->setPreviewSummary(summary);
        notification->setPreviewBody(fileName);
        notification->setSummary(summary);
    }
    notification->setHintValue("x-nemo-progress", job->progress());
    if (device) {
        notification->setRemoteActions(
            { UI::openDevicePageDbusAction(device->id()) });
    }

    notification->publish();

    m_jobs.insert(job, notification);
    connectJob(job);
}

} // namespace SailfishConnect
