#include "downloadjob.h"

#include <QFileInfo>
#include <QDir>
#include <QFile>

#include "corelogging.h"
#include <utils/filehelper.h>

namespace SailfishConnect {

DownloadJob::DownloadJob(const QSharedPointer<QIODevice>& origin,
        const QString& destination, qint64 size, QObject* parent)
    : CopyJob(origin, QSharedPointer<QIODevice>(), size, parent),
      m_destination(destination)
{
    setTitle(tr("Receiving file"));
    setDescription(QFileInfo(m_destination).fileName());

    if (QFileInfo::exists(m_destination)) {
        auto destination = nonexistingFile(m_destination);

        if (!destination.dir().mkpath(QStringLiteral("."))) {
            qCWarning(coreLogger)
                    << "Cannot create destination folder for file download"
                    << destination.filePath();
            abort(tr("Cannot create destination folder"));
            return;
        }

        m_destination = destination.filePath();

        qCInfo(coreLogger)
                << "Changed to non-existing destination" << m_destination;
    }

    QSharedPointer<QFile> file = QSharedPointer<QFile>(
                new QFile(m_destination));
    if (!file->open(QIODevice::WriteOnly | QIODevice::Unbuffered)) {
        abort(
            tr("Could not open file for writing: %1").arg(file->error()));
        return;
    }
    setDestination(std::move(file));
}

bool DownloadJob::doCancelling()
{
    close();
    return true;
}

void DownloadJob::onFinished()
{
    close();
    CopyJob::onFinished();
}

QString DownloadJob::destination() const
{
    return m_destination;
}

} // namespace SailfishConnect
