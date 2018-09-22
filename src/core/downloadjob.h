#ifndef DOWNLOADJOB_H
#define DOWNLOADJOB_H

#include "utils/copyjob.h"

namespace SailfishConnect {

class DownloadJob : public CopyJob
{
    Q_OBJECT
public:
    DownloadJob(
            const QSharedPointer<QIODevice>& origin,
            const QString &destination,
            qint64 size,
            QObject* parent = nullptr);

    QString destination() const;

protected:
    bool doCancelling() override;
    void onFinished() override;

private:
    QString m_destination;
};

} // namespace SailfishConnect

#endif // DOWNLOADJOB_H
