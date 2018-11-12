#ifndef JOBMANAGER_H
#define JOBMANAGER_H

#include <QObject>
#include "job.h"

class Device;


namespace SailfishConnect {

class JobInfo : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QString description READ description NOTIFY descriptionChanged)
    Q_PROPERTY(QString destination READ destination NOTIFY destinationChanged)
    Q_PROPERTY(qint64 totalBytes READ totalBytes NOTIFY totalBytesChanged)
    Q_PROPERTY(qint64 processedBytes READ processedBytes NOTIFY processedBytesChanged)
    Q_PROPERTY(Job::State state READ state NOTIFY stateChanged)

public:
    JobInfo(Job* job, QString deviceId, QObject* parent);

    QString title() const;
    QString description() const;
    QString destination() const { return m_destination; }
    qint64 totalBytes() const;
    qint64 processedBytes() const;
    QString deviceId() const { return m_deviceId; }

    Job::State state() const;

    void cancel();

signals:
    void titleChanged();
    void descriptionChanged();
    void destinationChanged();
    void totalBytesChanged();
    void processedBytesChanged();
    void stateChanged();

private:
    Job* m_impl = nullptr;

    QString m_title;
    QString m_description;
    QString m_destination;
    qint64 m_totalBytes;
    qint64 m_processedBytes;
    Job::State m_state;
    QString m_deviceId;

    void onJobDestroyed();
    void onJobFinished();
};

class JobManager : public QObject
{
    Q_OBJECT

public:
    JobManager(QObject* parent);

    QList<JobInfo*> jobs() const { return m_jobs; }

    void addJob(Job* job, const QString& deviceId);
    void removeJob(JobInfo* job);

signals:
    void jobAdded(JobInfo*);
    void jobRemoved(JobInfo*);

private:
    QList<JobInfo*> m_jobs;
};

} // namespace SailfishConnect

#endif // JOBMANAGER_H
