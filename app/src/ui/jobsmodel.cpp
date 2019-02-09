#include "jobsmodel.h"

#include <sailfishconnect/helper/cpphelper.h>
#include <sailfishconnect/daemon.h>

namespace SailfishConnect {

JobsModel::JobsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    setModel(Daemon::instance()->jobManager());
}

int JobsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_jobs.length();
}

QHash<int, QByteArray> JobsModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles.insert(TargetRole, "target");
    roles.insert(ActionRole, "action");
    roles.insert(ProcessedBytesRole, "processedBytes");
    roles.insert(TotalBytesRole, "totalBytes");
    roles.insert(StateRole, "currentState");
    roles.insert(CanceledRole, "canceled");
    roles.insert(ErrorRole, "error");
    roles.insert(DeviceIdRole, "deviceId");
    return roles;
}

void JobsModel::setModel(JobManager *jobMgr)
{
    beginResetModel();

    if (m_jobManager) {
        disconnect(jobMgr);
        for (auto* job : asConst(m_jobs)) {
            disconnect(job);
        }
    }
    m_jobs.clear();

    m_jobManager = jobMgr;

    if (m_jobManager) {
        connect(m_jobManager, &QObject::destroyed,
                this, [this](){ setModel(nullptr); });
        connect(m_jobManager, &JobManager::jobAdded,
                this, &JobsModel::addJob);

        m_jobs = m_jobManager->jobs();
        for (auto job : asConst(m_jobs)) {
            connectJob(job);
        }
    }

    endResetModel();
}

void JobsModel::addJob(JobInfo *job)
{
    beginInsertRows(QModelIndex(), 0, 0);
    m_jobs.append(job);
    connectJob(job);
    endInsertRows();
}

void JobsModel::connectJob(JobInfo *job)
{
    connect(job, &JobInfo::targetChanged,
            this, [=](){ jobChanged(job, TargetRole); });
    connect(job, &JobInfo::actionChanged,
            this, [=](){ jobChanged(job, ActionRole); });
    connect(job, &JobInfo::processedBytesChanged,
            this, [=](){ jobChanged(job, ProcessedBytesRole); });
    connect(job, &JobInfo::totalBytesChanged,
            this, [=](){ jobChanged(job, TotalBytesRole); });
    connect(job, &JobInfo::stateChanged,
            this, [=](){
        QVector<int> roles { StateRole };
        if (!job->errorString().isEmpty()) {
            roles.append(ErrorRole);
        }
        if (job->canceled()) {
            roles.append(CanceledRole);
        }
        jobChanged(job, roles);
    });
}

void JobsModel::jobChanged(JobInfo *job, const QVector<int>& roles)
{
    int row = jobToRow(job);
    if (row >= 0) {
        auto i = index(row);
        emit dataChanged(i, i, roles);
    }
}

void JobsModel::jobChanged(JobInfo *job, int role)
{
    jobChanged(job, QVector<int> { role });
}

JobInfo *JobsModel::rowToJob(const QModelIndex &index) const
{
    if (index.row() < 0 || index.row() >= m_jobs.length())
        return nullptr;

    return m_jobs[m_jobs.length() - index.row() - 1];
}

int JobsModel::jobToRow(JobInfo *job) const
{
    int row = m_jobs.indexOf(job);
    if (row >= 0) {
        return m_jobs.length() - row - 1;
    } else {
        return -1;
    }
}

QVariant JobsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    auto* job = rowToJob(index);
    if (!job)
        return QVariant();

    switch (role) {
    case TargetRole:
        return job->target();
    case ActionRole:
        return job->action();
    case ProcessedBytesRole:
        return job->processedBytes();
    case TotalBytesRole:
        return job->totalBytes();
    case StateRole:
        return int(job->state());
    case CanceledRole:
        return job->canceled();
    case ErrorRole:
        return job->errorString();
    case DeviceIdRole:
        return job->deviceId();
    }

    return QVariant();
}

bool JobsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
//    if (data(index, role) != value) {
//        emit dataChanged(index, index, QVector<int>() << role);
//        return true;
//    }
    return false;
}

Qt::ItemFlags JobsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::NoItemFlags;
    // return Qt::ItemIsEditable;
}

} // namespace SailfishConnect
