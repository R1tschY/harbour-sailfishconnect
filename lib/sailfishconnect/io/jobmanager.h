#ifndef JOBMANAGER_H
#define JOBMANAGER_H

#include <QObject>
#include "job.h"

class Device;


namespace SailfishConnect {

class JobInfo : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString deviceId READ deviceId CONSTANT)
    Q_PROPERTY(QUrl target READ target NOTIFY targetChanged)
    Q_PROPERTY(QString action READ action NOTIFY actionChanged)
    Q_PROPERTY(qint64 totalBytes READ totalBytes NOTIFY totalBytesChanged)
    Q_PROPERTY(qint64 processedBytes READ processedBytes NOTIFY processedBytesChanged)
    Q_PROPERTY(Job::State state READ state NOTIFY stateChanged)
    Q_PROPERTY(bool canceled READ canceled NOTIFY stateChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY stateChanged)

public:
    JobInfo(Job* job, QObject* parent);

    QUrl target() const;
    QString action() const;
    qint64 totalBytes() const;
    qint64 processedBytes() const;
    QString deviceId() const { return m_deviceId; }

    Job::State state() const;
    bool canceled() const;
    QString errorString() const;

    void cancel();

signals:
    void targetChanged();
    void actionChanged();
    void totalBytesChanged();
    void processedBytesChanged();
    void stateChanged();

private:
    Job* m_impl = nullptr;

    QUrl m_target;
    QString m_action;
    qint64 m_totalBytes;
    qint64 m_processedBytes;
    Job::State m_state;
    QString m_deviceId;
    bool m_canceled;
    QString m_errorString;

    void onJobDestroyed();
    void onStateChanged();
    void onTargetChanged();
};

class JobManager : public QObject
{
    Q_OBJECT

public:
    JobManager(QObject* parent);

    QList<JobInfo*> jobs() const { return m_jobs; }

    void addJob(Job* job);
    void removeJob(JobInfo* job);

signals:
    void jobAdded(SailfishConnect::JobInfo*);
    void jobRemoved(SailfishConnect::JobInfo*);

private:
    QList<JobInfo*> m_jobs;
};

} // namespace SailfishConnect

#endif // JOBMANAGER_H
