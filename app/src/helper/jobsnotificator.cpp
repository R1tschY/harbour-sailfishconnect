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

#include <QCoreApplication>
#include <KJobTrackerInterface>
#include <QLoggingCategory>
#include <KLocalizedString>

#include <core/daemon.h>
#include <core/device.h>
#include <notification.h>

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
    connect(job, &JobInfo::stateChanged, this, callback);
}

void JobsNotificator::jobChanged(JobInfo *job)
{
    bool isUpload = job->target().scheme() == QStringLiteral("remote");

    Notification* notification = m_jobs.value(job);
    Q_ASSERT(notification != nullptr);

    if (job->state() == QStringLiteral("running")) {
        notification->setHintValue("x-nemo-progress", job->progress());

        if (isUpload) {
            notification->setBody(i18n("Uploading ..."));
        } else {
            notification->setBody(i18n("Downloading ..."));
        }
    } else {
        QString fileName = job->target().fileName();
        quint32 id = notification->replacesId();
        notification->deleteLater();

        // recreate to get rid of the progress bar
        notification = new Notification(this);
        m_jobs[job] = notification;
        notification->setAppName(QCoreApplication::applicationName());
        notification->setSummary(fileName);
        notification->setReplacesId(id);

        if (job->state() == QStringLiteral("finished")) {
            if (job->errorString().isEmpty()) {
                if (isUpload) {
                    notification->setBody(i18n("Upload succedded"));
                } else {
                    notification->setBody(i18n("Download succedded"));
                }
            } else {
                if (isUpload) {
                    notification->setSummary(i18n("Upload failed"));
                    notification->setBody(
                        i18n("Upload failed: %1").arg(job->errorString()));
                } else {
                    notification->setSummary(i18n("Download failed"));
                    notification->setBody(
                        i18n("Download failed: %1").arg(job->errorString()));
                }
            }

            notification->setPreviewSummary(notification->body());
            notification->setPreviewBody(fileName);
        } else if (job->state() == QStringLiteral("canceled")) {
            if (isUpload) {
                notification->setBody(i18n("Upload canceled"));
            } else {
                notification->setBody(i18n("Download canceled"));
            }
        } else {
            return;
        }
    }
    notification->publish();
}

void JobsNotificator::addJob(JobInfo *job)
{
    bool isUpload = job->target().scheme() == QStringLiteral("remote");
    QString fileName = job->target().fileName();
    Device* device = Daemon::instance()->getDevice(job->deviceId());
    QString deviceName = device ? device->name() : QStringLiteral("?");

    Notification* notification = new Notification(this);
    notification->setAppName(QCoreApplication::applicationName());
    notification->setSummary(fileName);
    notification->setBody(i18n("Pending upload ..."));
    if (!isUpload) {
        notification->setPreviewSummary(i18n("Download from %1").arg(deviceName));
        notification->setPreviewBody(fileName);
    }
    notification->setHintValue("x-nemo-progress", job->progress());
//    TODO: notification->setRemoteActions(
//        { UI::openDevicePageDbusAction(device->id()) });

    notification->publish();

    m_jobs.insert(job, notification);
    connectJob(job);
}

} // namespace SailfishConnect
