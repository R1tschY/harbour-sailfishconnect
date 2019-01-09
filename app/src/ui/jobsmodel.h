#ifndef SAILFISHCONNECT_JOBSMODEL_H
#define SAILFISHCONNECT_JOBSMODEL_H

#include <QAbstractListModel>
#include <sailfishconnect/io/jobmanager.h>

namespace SailfishConnect {

class JobsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum ExtraRoles {
        TitleRole = Qt::UserRole,
        DescriptionRole,
        DestinationRole,
        ProcessedBytesRole,
        TotalBytesRole,
        StateRole,
        CanceledRole,
        ErrorRole,
    };

    explicit JobsModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    QHash<int, QByteArray> roleNames() const;

private:
    QList<JobInfo*> m_jobs;
    JobManager* m_jobManager = nullptr;

    void setModel(JobManager* jobMgr);

    void addJob(JobInfo* job);
    void connectJob(JobInfo* job);
    void jobChanged(JobInfo *job, int role);
    void jobChanged(JobInfo *job, const QVector<int>& roles);

    JobInfo* rowToJob(const QModelIndex& row) const;
    int jobToRow(JobInfo* job) const;
};

} // namespace SailfishConnect

#endif // SAILFISHCONNECT_JOBSMODEL_H
