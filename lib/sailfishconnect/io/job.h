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

#ifndef JOB_H
#define JOB_H

#include <QObject>
#include <QUrl>
#include <QString>
#include <QElapsedTimer>

namespace SailfishConnect {

class Job : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString errorString READ errorString NOTIFY stateChanged)
    Q_PROPERTY(QUrl target READ target NOTIFY targetChanged)
    Q_PROPERTY(QString action READ action NOTIFY actionChanged)
    Q_PROPERTY(qint64 totalBytes READ totalBytes NOTIFY totalBytesChanged)
    Q_PROPERTY(qint64 processedBytes READ processedBytes NOTIFY processedBytesChanged)
    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(bool canceled READ canceled NOTIFY stateChanged)
public:
    enum class State {
        Pending, Running, Finished
    };
    Q_ENUM(State)

    explicit Job(const QString &deviceId, QObject* parent = nullptr);
    ~Job();

    QString deviceId() const { return m_deviceId; }
    /// \brief an uri to the target file.
    ///
    /// possibilities:
    ///   local:/home/user/Downloads/file.ext
    ///   remote:file.ext
    QUrl target() const { return m_target; }
    QString action() const { return m_action; }
    qint64 totalBytes() const { return m_totalBytes; }
    qint64 processedBytes() const { return m_processedBytes; }
    State state() const { return m_state; }
    bool canceled() const { return m_canceled; }
    QString errorString() const { return m_errorString; }
    Q_SCRIPTABLE bool isPending() const { return m_state == State::Pending; }
    Q_SCRIPTABLE bool isRunning() const { return m_state == State::Running; }
    Q_SCRIPTABLE bool isFinished() const { return m_state == State::Finished; }


public slots:
    void start();
    void cancel();

signals:
    void finished();
    void success();
    void error();

    void targetChanged();
    void actionChanged();
    void totalBytesChanged();
    void processedBytesChanged();
    void stateChanged();

protected:
    virtual void doStart() = 0;
    virtual bool doCancelling();

    virtual void onFinished();
    virtual void onSuccess();
    virtual void onError();

    void setTarget(const QUrl &target);
    void setErrorString(const QString& errorString);
    void setAction(const QString& action);
    void setTotalBytes(qint64 totalBytes);
    void setProcessedBytes(qint64 processedBytes);

    void exit();

    /**
     * @brief convenience function to exit job with a error
     * @param error error message
     */
    void abort(const QString& error);

private:
    QString m_errorString;
    QString m_deviceId;
    QUrl m_target;
    QString m_action;
    qint64 m_totalBytes = -1;
    qint64 m_processedBytes = 0;
    QElapsedTimer m_timer;

    State m_state = State::Pending;
    bool m_canceled = false;
};

} // namespace SailfishConnect

#endif // JOB_H
