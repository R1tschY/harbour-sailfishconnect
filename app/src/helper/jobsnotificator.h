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

#ifndef JOBSNOTIFICATOR_H
#define JOBSNOTIFICATOR_H

#include "../io/jobmanager.h"
#include <QObject>
#include <QHash>

class Notification;

namespace SailfishConnect {

class JobsNotificator : public QObject
{
public:
    JobsNotificator(JobManager* jobManager);

    void connectJob(JobInfo *job);
    void jobChanged(JobInfo *job);
    void addJob(JobInfo *job);

private:
    QHash<JobInfo*, Notification*> m_jobs;
    JobManager* m_jobManager = nullptr;
};

} // namespace SailfishConnect

#endif // JOBSNOTIFICATOR_H
