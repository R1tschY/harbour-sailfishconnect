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
    roles.insert(TitleRole, "title");
    roles.insert(DescriptionRole, "description");
    roles.insert(DestinationRole, "destination");
    roles.insert(ProcessedBytesRole, "processedBytes");
    roles.insert(TotalBytesRole, "totalBytes");
    roles.insert(RunningRole, "running");
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
    connect(job, &JobInfo::titleChanged,
            this, [=](){ jobChanged(job, TitleRole); });
    connect(job, &JobInfo::descriptionChanged,
            this, [=](){ jobChanged(job, DescriptionRole); });
    connect(job, &JobInfo::destinationChanged,
            this, [=](){ jobChanged(job, DestinationRole); });
    connect(job, &JobInfo::processedBytesChanged,
            this, [=](){ jobChanged(job, ProcessedBytesRole); });
    connect(job, &JobInfo::totalBytesChanged,
            this, [=](){ jobChanged(job, TotalBytesRole); });
}

void JobsModel::jobChanged(JobInfo *job, int role)
{
    int row = m_jobs.indexOf(job);
    if (row > 0) {
        auto i = index(row);
        emit dataChanged(i, i, QVector<int>() << role);
    }
}

QVariant JobsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_jobs.length())
        return QVariant();

    auto* job = m_jobs[m_jobs.length() - index.row() - 1];
    switch (role) {
    case TitleRole:
        return job->title();
    case DescriptionRole:
        return job->description();
    case DestinationRole:
        return job->destination();
    case ProcessedBytesRole:
        return job->processedBytes();
    case TotalBytesRole:
        return job->totalBytes();
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
