#ifndef DOWNLOADJOB_H
#define DOWNLOADJOB_H

#include <QString>
#include <sailfishconnect/io/copyjob.h>


namespace SailfishConnect {

class DownloadJob : public CopyJob
{
    Q_OBJECT
public:
    DownloadJob(
            const QString &deviceId,
            const QSharedPointer<QIODevice>& origin,
            const QString &destination,
            qint64 size,
            QObject* parent = nullptr);

    QString destination() const;

protected:
    void doStart() override;
    bool doCancelling() override;
    void onFinished() override;
    void onError() override;

private:
    QString m_destination;
};

} // namespace SailfishConnect

#endif // DOWNLOADJOB_H
