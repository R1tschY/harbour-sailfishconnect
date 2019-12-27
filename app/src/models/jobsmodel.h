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

// #ifndef SAILFISHCONNECT_JOBSMODEL_H
// #define SAILFISHCONNECT_JOBSMODEL_H

// #include <QAbstractListModel>
// #include <sailfishconnect/io/jobmanager.h>

// namespace SailfishConnect {

// class JobsModel : public QAbstractListModel
// {
//     Q_OBJECT

// public:
//     enum ExtraRoles {
//         TitleRole = Qt::UserRole,
//         TargetRole,
//         ProcessedBytesRole,
//         TotalBytesRole,
//         StateRole,
//         ErrorRole,
//         DeviceIdRole,
//     };

//     explicit JobsModel(QObject *parent = nullptr);

//     int rowCount(const QModelIndex &parent = QModelIndex()) const override;

//     QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

//     bool setData(const QModelIndex &index, const QVariant &value,
//                  int role = Qt::EditRole) override;

//     Qt::ItemFlags flags(const QModelIndex& index) const override;

//     QHash<int, QByteArray> roleNames() const;

// private:
//     QList<JobInfo*> m_jobs;
//     JobManager* m_jobManager = nullptr;

//     void setModel(JobManager* jobMgr);

//     void addJob(JobInfo* job);
//     void connectJob(JobInfo* job);
//     void jobChanged(JobInfo *job, int role);
//     void jobChanged(JobInfo *job, const QVector<int>& roles);

//     JobInfo* rowToJob(const QModelIndex& row) const;
//     int jobToRow(JobInfo* job) const;
// };

// } // namespace SailfishConnect

// #endif // SAILFISHCONNECT_JOBSMODEL_H
