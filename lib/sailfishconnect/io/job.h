#ifndef JOB_H
#define JOB_H

#include <QObject>

namespace SailfishConnect {

class Job;


class JobManager : public QObject
{
    Q_OBJECT

    JobManager();
public:
    static JobManager* instance();

    QList<Job*> jobs() const;

    void addJob(Job* job);

signals:
    void jobAdded(Job*);
    void jobRemoved(Job*);
};


class Job : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QString description READ description NOTIFY descriptionChanged)
    Q_PROPERTY(qint64 totalBytes READ totalBytes NOTIFY totalBytesChanged)
    Q_PROPERTY(qint64 processedBytes READ processedBytes NOTIFY processedBytesChanged)
public:
    enum class State {
        Pending, Running, Finished
    };

    explicit Job(QObject* parent = nullptr);
    ~Job();

    QString errorString() const { return m_errorString; }
    QString title() const { return m_title; }
    QString description() const { return m_description; }
    qint64 totalBytes() const { return m_totalBytes; }
    qint64 processedBytes() const { return m_processedBytes; }
    Q_SCRIPTABLE bool wasCancelled() const { return m_wasCancelled; }
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

    void errorStringChanged();
    void titleChanged();
    void descriptionChanged();
    void totalBytesChanged();
    void processedBytesChanged();

protected:
    virtual void doStart() = 0;
    virtual bool doCancelling();

    virtual void onFinished();
    virtual void onSuccess();
    virtual void onError();

    void setTitle(const QString& title);
    void setErrorString(const QString& errorString);
    void setDescription(const QString& description);
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
    QString m_title;
    QString m_description;
    qint64 m_totalBytes = -1;
    qint64 m_processedBytes = 0;

    State m_state = State::Pending;
    bool m_wasCancelled = false;
};

} // namespace SailfishConnect

#endif // JOB_H
