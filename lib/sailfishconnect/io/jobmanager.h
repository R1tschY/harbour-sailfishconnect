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

#ifndef JOBMANAGER_H
#define JOBMANAGER_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QPair>
#include <KJobTrackerInterface>

class Device;

namespace SailfishConnect {

class JobInfo : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QUrl target READ target NOTIFY targetChanged)
    Q_PROPERTY(QString deviceId READ deviceId CONSTANT)
    Q_PROPERTY(qulonglong totalBytes
               READ totalBytes NOTIFY totalBytesChanged)
    Q_PROPERTY(qulonglong processedBytes
               READ processedBytes NOTIFY processedBytesChanged)
    Q_PROPERTY(QString state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY stateChanged)

public:
    JobInfo(KJob* job, QObject* parent);

    qint64 totalBytes() const { return m_totalBytes; }
    qint64 processedBytes() const { return m_processedBytes; }
    QString deviceId() const { return m_deviceId; }

    QString title() const { return m_title; }
    QUrl target() const { return m_target; }
    QString state() const { return m_state; }
    QString errorString() const { return m_errorString; }

    void cancel();

    KJob* job() const { return m_impl; }

signals:
    void titleChanged();
    void targetChanged();
    void totalBytesChanged();
    void processedBytesChanged();
    void stateChanged();

private:
    KJob* m_impl = nullptr;

    QString m_state;
    QString m_deviceId;
    QUrl m_target;

    QString m_errorString;
    qulonglong m_totalBytes;
    qulonglong m_processedBytes;

    QString m_title;
    QPair<QString, QString> m_field1;
    QPair<QString, QString> m_field2;

    void getTarget();
    void setTarget(const QUrl& value);

private slots:
    void onResult();
    void onFinished();
    void onDescription(
            KJob *job,
            const QString &title,
            const QPair<QString, QString> &field1,
            const QPair<QString, QString> &field2);
    void onTotalAmount(KJob *job, KJob::Unit unit, qulonglong amount);
    void onProcessedAmount(KJob *job, KJob::Unit unit, qulonglong amount);
};

class JobManager : public KJobTrackerInterface
{
    Q_OBJECT

public:
    JobManager(QObject* parent);

    QList<JobInfo*> jobs() const { return m_jobs; }

    void registerJob(KJob *job) override;
    void unregisterJob(KJob *job) override;

signals:
    void jobAdded(SailfishConnect::JobInfo*);
    void jobRemoved(SailfishConnect::JobInfo*);

private:
    QList<JobInfo*> m_jobs;

};

} // namespace SailfishConnect

#endif // JOBMANAGER_H
