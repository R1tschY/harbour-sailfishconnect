// /*
//  * Copyright 2019 Richard Liebscher <richard.liebscher@gmail.com>.
//  *
//  * This program is free software: you can redistribute it and/or modify
//  * it under the terms of the GNU General Public License as published by
//  * the Free Software Foundation, either version 3 of the License, or
//  * (at your option) any later version.
//  *
//  * This program is distributed in the hope that it will be useful,
//  * but WITHOUT ANY WARRANTY; without even the implied warranty of
//  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  * GNU General Public License for more details.
//  *
//  * You should have received a copy of the GNU General Public License
//  * along with this program.  If not, see <http://www.gnu.org/licenses/>.
//  */

// #include "jobsmodel.h"

// #include <sailfishconnect/helper/cpphelper.h>
// #include <sailfishconnect/daemon.h>

// namespace SailfishConnect {

// JobsModel::JobsModel(QObject *parent)
//     : QAbstractListModel(parent)
// {
//     setModel(Daemon::instance()->jobManager());
// }

// int JobsModel::rowCount(const QModelIndex &parent) const
// {
//     if (parent.isValid())
//         return 0;

//     return m_jobs.length();
// }

// QHash<int, QByteArray> JobsModel::roleNames() const
// {
//     QHash<int, QByteArray> roles;
//     roles.insert(TitleRole, "title");
//     roles.insert(TargetRole, "target");
//     roles.insert(ProcessedBytesRole, "processedBytes");
//     roles.insert(TotalBytesRole, "totalBytes");
//     roles.insert(StateRole, "currentState");
//     roles.insert(ErrorRole, "error");
//     roles.insert(DeviceIdRole, "deviceId");
//     return roles;
// }

// void JobsModel::setModel(JobManager *jobMgr)
// {
//     beginResetModel();

//     if (m_jobManager) {
//         disconnect(jobMgr);
//         for (auto* job : asConst(m_jobs)) {
//             disconnect(job);
//         }
//     }
//     m_jobs.clear();

//     m_jobManager = jobMgr;

//     if (m_jobManager) {
//         connect(m_jobManager, &QObject::destroyed,
//                 this, [this](){ setModel(nullptr); });
//         connect(m_jobManager, &JobManager::jobAdded,
//                 this, &JobsModel::addJob);

//         m_jobs = m_jobManager->jobs();
//         for (auto job : asConst(m_jobs)) {
//             connectJob(job);
//         }
//     }

//     endResetModel();
// }

// void JobsModel::addJob(JobInfo *job)
// {
//     beginInsertRows(QModelIndex(), 0, 0);
//     m_jobs.append(job);
//     connectJob(job);
//     endInsertRows();
// }

// void JobsModel::connectJob(JobInfo *job)
// {
//     connect(job, &JobInfo::processedBytesChanged,
//             this, [=](){ jobChanged(job, ProcessedBytesRole); });
//     connect(job, &JobInfo::totalBytesChanged,
//             this, [=](){ jobChanged(job, TotalBytesRole); });
//     connect(job, &JobInfo::stateChanged,
//             this, [=](){
//         jobChanged(job, { StateRole, ErrorRole });
//     });
// }

// void JobsModel::jobChanged(JobInfo *job, const QVector<int>& roles)
// {
//     int row = jobToRow(job);
//     if (row >= 0) {
//         auto i = index(row);
//         emit dataChanged(i, i, roles);
//     }
// }

// void JobsModel::jobChanged(JobInfo *job, int role)
// {
//     jobChanged(job, QVector<int> { role });
// }

// JobInfo *JobsModel::rowToJob(const QModelIndex &index) const
// {
//     if (index.row() < 0 || index.row() >= m_jobs.length())
//         return nullptr;

//     return m_jobs[m_jobs.length() - index.row() - 1];
// }

// int JobsModel::jobToRow(JobInfo *job) const
// {
//     int row = m_jobs.indexOf(job);
//     if (row >= 0) {
//         return m_jobs.length() - row - 1;
//     } else {
//         return -1;
//     }
// }

// QVariant JobsModel::data(const QModelIndex &index, int role) const
// {
//     if (!index.isValid())
//         return QVariant();

//     auto* job = rowToJob(index);
//     if (!job)
//         return QVariant();

//     switch (role) {
//     case TitleRole:
//         return job->title();
//     case TargetRole:
//         return job->target();
//     case ProcessedBytesRole:
//         return job->processedBytes();
//     case TotalBytesRole:
//         return job->totalBytes();
//     case StateRole:
//         return job->state();
//     case ErrorRole:
//         return job->errorString();
//     case DeviceIdRole:
//         return job->deviceId();
//     }

//     return QVariant();
// }

// bool JobsModel::setData(const QModelIndex &index, const QVariant &value, int role)
// {
// //    if (data(index, role) != value) {
// //        emit dataChanged(index, index, QVector<int>() << role);
// //        return true;
// //    }
//     return false;
// }

// Qt::ItemFlags JobsModel::flags(const QModelIndex &index) const
// {
//     if (!index.isValid())
//         return Qt::NoItemFlags;

//     return Qt::NoItemFlags;
//     // return Qt::ItemIsEditable;
// }

// } // namespace SailfishConnect
