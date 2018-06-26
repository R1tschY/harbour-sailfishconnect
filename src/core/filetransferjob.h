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

#ifndef FILETRANSFERJOB_H
#define FILETRANSFERJOB_H

#include <QElapsedTimer>
#include <QIODevice>
#include <QSharedPointer>
#include <QUrl>
#include <QNetworkReply>

#include "../utils/job.h"

/**
 * @short It will stream a device into a url destination
 *
 * Given a QIODevice, the file transfer job will use the system's QNetworkAccessManager
 * for putting the stream into the requested location.
 */
class FileTransferJob : public SailfishConnect::Job
{
    Q_OBJECT

public:
    /**
     * @p origin specifies the data to read from.
     * @p size specifies the expected size of the stream we're reading.
     * @p destination specifies where these contents should be stored
     */
    FileTransferJob(const QSharedPointer<QIODevice>& origin, qint64 size, const QUrl& destination, QObject *parent = Q_NULLPTR);
    ~FileTransferJob();

    void doStart() override;
    QUrl destination() const { return m_destination; }

protected:
    void close();
    bool doCancelling() override;

private:
    void startTransfer();
    void transferFailed(QNetworkReply::NetworkError error);
    void transferFinished();

    QSharedPointer<QIODevice> m_origin;
    QNetworkReply* m_reply;
    QUrl m_destination;
    QElapsedTimer m_timer;
    qulonglong m_speedBytes;
    qint64 m_written;
    qint64 m_size;
};

#endif
