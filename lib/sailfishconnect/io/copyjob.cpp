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

#include "copyjob.h"

#include <QIODevice>
#include <QLoggingCategory>
#include <QTimer>
#include <QFile>
#include <QSslSocket>
#include <algorithm>

namespace SailfishConnect {

static Q_LOGGING_CATEGORY(logger, "sailfishconnect.io")

CopyJob::CopyJob(const QString& deviceId,
        const QSharedPointer<QIODevice>& source,
        const QSharedPointer<QIODevice>& destination,
        qint64 size,
        QObject *parent)
    : KJob(parent)
    , m_source(source)
    , m_destination(destination)
    , m_buffer()
    , m_size(size)
    , m_deviceId(deviceId)
{
    m_timer.setInterval(100);
    m_timer.setSingleShot(false);
    connect(&m_timer, &QTimer::timeout, this, &CopyJob::poll);

    setCapabilities(KJob::Killable);
}

void CopyJob::close()
{
    m_source->close();
    m_destination->close();
}

void CopyJob::setDestination(const QSharedPointer<QIODevice> &destination)
{
    if (m_started)
        return;

    m_destination = destination;
}

void CopyJob::start()
{
    QMetaObject::invokeMethod(this, "doStart", Qt::QueuedConnection);
}

void CopyJob::setSource(const QSharedPointer<QIODevice> &source)
{
    if (m_started)
        return;

    m_source = source;
}

void CopyJob::doStart()
{
    qCDebug(logger) << "CopyJob::doStart";
    Q_ASSERT(!m_source.isNull());
    Q_ASSERT(!m_destination.isNull());

    if (!m_source->isOpen() || !m_source->isReadable()) {
        setError(2);
        setErrorText(tr("Input stream is not readable."));
        return emitResult();
    }
    if (!m_destination->isOpen() || !m_destination->isWritable()) {
        setError(2);
        setErrorText(tr("Output stream is not writable."));
        return emitResult();
    }
    if (!m_source->isSequential()) {
        m_sourceEof = true;
    }

    m_sslSocket = qobject_cast<QSslSocket*>(m_destination.data());

    m_started = true;

    m_size = (m_size >= 0)
            ? m_size : (!m_source->isSequential()) ? m_source->size() : -1;
    setTotalAmount(KJob::Bytes, m_size);

    connect(m_source.data(), &QIODevice::readChannelFinished,
            this, &CopyJob::pollAtSourceClose);
    connect(m_destination.data(), &QIODevice::aboutToClose,
            this, &CopyJob::pollAtDestinationClose);

    poll();
    m_timer.start();
}

QString CopyJob::deviceId() const
{
    return m_deviceId;
}

void CopyJob::poll()
{
    if (m_finished)
        return;

    if (m_bufferSize < m_buffer.size() && bytesToWrite() < 1024 * 1024) {
        qint64 bytes = m_source->read(
                    m_buffer.data() + m_bufferSize, m_buffer.size() - m_bufferSize);
        if (bytes == -1) {
            // read error
            setError(2);
            setErrorText(tr("Read error: %1").arg(m_source->errorString()));
            return emitResult();
        }
        m_bufferSize += bytes;
//        qCDebug(logger)
//                << "Read" << bytes
//                << "bytes. Buffer size:" << m_bufferSize;
    }

    if (m_bufferSize != 0) {
        qint64 bytes = m_destination->write(m_buffer.data(), m_bufferSize);
        if (bytes == -1) {
            // write error
            setError(2);
            setErrorText(
                tr("Write error: %1").arg(m_destination->errorString()));
            return emitResult();
        }
        std::move(m_buffer.begin() + bytes, m_buffer.begin() + m_bufferSize,
                  m_buffer.begin());
        m_bufferSize -= bytes;
        m_writtenBytes += bytes;

//        qCDebug(logger)
//                << "Written" << bytes
//                << "bytes. Buffer size:" << m_bufferSize
//                << "Waiting for" << bytesToWrite() << "bytes";
    }

    auto btw = bytesToWrite();
    setProcessedAmount(KJob::Bytes, m_writtenBytes - btw);

    if (m_source->bytesAvailable() > 0
            && m_bufferSize != m_buffer.size()
            && btw < 1024 * 1024) // 1 MB
    {
        QMetaObject::invokeMethod(this, "poll", Qt::QueuedConnection);
    }

    if (m_sourceEof
            && m_bufferSize == 0
            && m_source->bytesAvailable() == 0
            && btw == 0) {
        qCDebug(logger) << "EOF";
        finish();
    }
}

void CopyJob::pollAtSourceClose()
{
    if (m_finished)
        return;

    qCDebug(logger) << "Detected source closing";

    m_sourceEof = true;
    poll();
}

void CopyJob::pollAtDestinationClose()
{
    if (!m_finished) {
        qCDebug(logger) << "Detected destination closing";
        finish();
    }
}

void CopyJob::finish()
{
    m_finished = true;
    m_timer.stop();

    if (m_bufferSize != 0) {
        setError(2);
        setErrorText(tr("Early end of output stream"));
    }

    if (m_size > 0 && m_writtenBytes > m_size) {
        setError(2);
        setErrorText(tr("Read more bytes of input stream than "
                        "expected."));
    }

    if (m_size > 0 && m_writtenBytes < m_size) {
        setError(2);
        setErrorText(tr("Early end of input stream"));
    }

    close();

    // success
    emitResult();
}

qint64 CopyJob::bytesToWrite() const
{
    if (m_sslSocket) {
        return m_destination->bytesToWrite() +
                m_sslSocket->encryptedBytesToWrite();
    } else {
        return m_destination->bytesToWrite();
    }
}

bool CopyJob::doKill()
{
    close();
    return true;
}

} // namespace SailfishConnect

