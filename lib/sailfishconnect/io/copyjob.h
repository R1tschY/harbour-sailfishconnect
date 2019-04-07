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

#pragma once

#include <array>

#include <QSharedPointer>
#include <QString>
#include <QTimer>
#include <KJob>

class QIODevice;
class QSslSocket;

namespace SailfishConnect {

class CopyJob : public KJob
{
    Q_OBJECT
public:
    /**
     * @brief Copy bytes from input stream to output stream
     * @param source input stream
     * @param destination output stream
     * @param size expected number of bytes of input stream or -1 if size is
     *  unknown.
     * @param parent parent Qt-Object
     *
     * IO-Devices have to be opened.
     * IO-Devices are not closed at destruction.
     */
    explicit CopyJob(
            const QString &deviceId,
            const QSharedPointer<QIODevice>& source,
            const QSharedPointer<QIODevice>& destination,
            qint64 size = -1,
            QObject *parent = nullptr);

    QSharedPointer<QIODevice> source() const { return m_source; }
    QSharedPointer<QIODevice> destination() const { return m_destination; }
    void setSource(const QSharedPointer<QIODevice> &source);
    void setDestination(const QSharedPointer<QIODevice> &destination);

    void start() override;

    QString deviceId() const;

protected:
    void close();
    bool doKill() override;

protected slots:
    virtual void doStart();

private:
    QSharedPointer<QIODevice> m_source;
    QSharedPointer<QIODevice> m_destination;
    QSslSocket* m_sslSocket = nullptr;


    qint64 m_size = -1;
    qint64 m_writtenBytes = 0;
    bool m_sourceEof = false;
    bool m_started = false;
    bool m_finished = false;
    QTimer m_timer;
    QString m_deviceId;

    std::size_t m_bufferSize = 0;
    std::array<char, 64 * 1024> m_buffer;

    void pollAtSourceClose();
    void pollAtDestinationClose();

    void closeSource();
    void closeDestination();

    void checkSource();
    void checkDestination();

    void finish();

    qint64 bytesToWrite() const;

private slots:
    void poll();
};

} // namespace SailfishConnect
