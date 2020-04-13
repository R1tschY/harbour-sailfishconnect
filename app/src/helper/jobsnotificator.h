#ifndef JOBSNOTIFICATOR_H
#define JOBSNOTIFICATOR_H

#include <sailfishconnect/io/jobmanager.h>
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
