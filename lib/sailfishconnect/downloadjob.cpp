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

#include "downloadjob.h"

#include <QFileInfo>
#include <QDir>
#include <QFile>

#include "corelogging.h"
#include <sailfishconnect/helper/filehelper.h>

namespace SailfishConnect {

DownloadJob::DownloadJob(
        const QString& deviceId,
        const QSharedPointer<QIODevice>& origin,
        const QString& destination, qint64 size, QObject* parent)
    : CopyJob(deviceId, origin, QSharedPointer<QIODevice>(), size, parent),
      m_destination(destination)
{
    QUrl target;
    target.setScheme(QStringLiteral("local"));
    target.setPath(destination);

    setTarget(target);
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

    target.setPath(m_destination);
    setTarget(target);
}

bool DownloadJob::doCancelling()
{
    close();
    QFile::remove(m_destination);
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

void DownloadJob::doStart()
{
    setAction(tr("Receiving"));
    CopyJob::doStart();
}

void DownloadJob::onError()
{
    QFile::remove(m_destination);
    Job::onError();
}

} // namespace SailfishConnect
