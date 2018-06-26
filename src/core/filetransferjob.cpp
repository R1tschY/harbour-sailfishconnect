/*
 * Copyright 2013 Albert Vaca <albertvaka@gmail.com>
 * Copyright 2015 Aleix Pol i Gonzalez <aleixpol@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/*
 * Copyright 2018 Richard Liebscher <richard.liebscher@gmail.com>.
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

#include "filetransferjob.h"
#include "daemon.h"
#include "corelogging.h"

#include <qalgorithms.h>
#include <QFileInfo>
#include <QDebug>

using namespace SailfishConnect;


FileTransferJob::FileTransferJob(const QSharedPointer<QIODevice>& origin, qint64 size, const QUrl& destination, QObject *parent)
    : Job()
    , m_origin(origin)
    , m_reply(Q_NULLPTR)
    , m_destination(destination)
    , m_speedBytes(0)
    , m_written(0)
    , m_size(size)
{
    Q_ASSERT(m_origin);
    Q_ASSERT(m_origin->isReadable());
    if (m_destination.scheme().isEmpty()) {
        qCWarning(coreLogger) << "Destination QUrl" << m_destination << "lacks a scheme. Setting its scheme to 'file'.";
        m_destination.setScheme(QStringLiteral("file"));
    }

    qCDebug(coreLogger) << "FileTransferJob Downloading payload to" << destination << "size:" << size;
}

FileTransferJob::~FileTransferJob()
{
    close();
}

void FileTransferJob::doStart()
{
    setTitle(tr("Receiving file over KDE Connect"));
    setDescription(m_destination.toLocalFile());

    if (m_destination.isLocalFile() && QFile::exists(m_destination.toLocalFile())) {
        setErrorString(tr("Filename already present"));
        exit();
        return;
    }

    if (m_origin->bytesAvailable())
        startTransfer();

    connect(
        m_origin.data(), &QIODevice::readyRead,
                this, &FileTransferJob::startTransfer);
}

void FileTransferJob::close()
{

}

void FileTransferJob::startTransfer()
{
    // Don't put each ready read
    if (m_reply)
        return;

    setProcessedBytes(0);

    QNetworkRequest req(m_destination);
    if (m_size >= 0) {
        setTotalBytes(m_size);
        req.setHeader(QNetworkRequest::ContentLengthHeader, m_size);
    }
    m_reply = Daemon::instance()->networkAccessManager()->put(req, m_origin.data());

    connect(m_reply, &QNetworkReply::uploadProgress, this, [this](qint64 bytesSent, qint64 /*bytesTotal*/) {
        setProcessedBytes(bytesSent);
    });
    connect(m_reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
            this, &FileTransferJob::transferFailed);
    connect(m_reply, &QNetworkReply::finished, this, &FileTransferJob::transferFinished);
}

void FileTransferJob::transferFailed(QNetworkReply::NetworkError error)
{
    qCDebug(coreLogger) << "Couldn't transfer the file successfully" << error << m_reply->errorString();
    setErrorString(tr("Received incomplete file: %1").arg(m_reply->errorString()));
    exit();

    m_reply->close();
}

void FileTransferJob::transferFinished()
{
    //TODO: MD5-check the file
    qCDebug(coreLogger) << "Finished transfer" << m_destination;

    exit();
}

bool FileTransferJob::doCancelling()
{
    if (m_reply) {
        m_reply->close();
        m_reply = nullptr;
    }
    if (m_origin) {
        m_origin->close();
        m_origin.reset();
    }
}
