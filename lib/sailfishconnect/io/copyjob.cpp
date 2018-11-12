#include "copyjob.h"

#include <QIODevice>
#include <QLoggingCategory>
#include <QTimer>

namespace SailfishConnect {

static Q_LOGGING_CATEGORY(logger, "sailfishconnect.io")

static qint64 maxBufferSize = 512 * 1024; // 512kB


CopyJob::CopyJob(
        QSharedPointer<QIODevice> source,
        QSharedPointer<QIODevice> destination,
        qint64 size,
        QObject *parent)
    : Job(parent),
      m_source(source), m_destination(destination), m_buffer(), m_size(size)
{ }

void CopyJob::close()
{
    cancel();

    if (m_source) {
        m_source->close();
    }

    if (m_destination) {
        m_destination->close();
    }
}

void CopyJob::setDestination(const QSharedPointer<QIODevice> &destination)
{
    if (m_started)
        return;

    m_destination = destination;
}

void CopyJob::setSource(const QSharedPointer<QIODevice> &source)
{
    if (m_started)
        return;

    m_source = source;
}

void CopyJob::doStart()
{
    Q_ASSERT(!m_source.isNull());
    Q_ASSERT(!m_destination.isNull());

    if (!m_source->isOpen() || !m_source->isReadable()) {
        return abort(tr("Input stream is not readable."));
    }
    if (!m_destination->isOpen() || !m_destination->isWritable()) {
        return abort(tr("Output stream is not writable."));
    }

    m_started = true;

    m_size = (m_size >= 0)
            ? m_size : (!m_source->isSequential()) ? m_source->size() : -1;
    setTotalBytes(m_size);

    connect(m_source.data(), &QIODevice::readChannelFinished,
            this, &CopyJob::pollAtSourceClose);
    connect(m_source.data(), &QIODevice::readyRead,
            this, &CopyJob::poll);
    connect(m_destination.data(), &QIODevice::bytesWritten,
            this, &CopyJob::pollBytesWritten);
    connect(m_destination.data(), &QIODevice::aboutToClose,
            this, &CopyJob::pollAtDestinationClose);

    poll();
}

void CopyJob::poll()
{
    if (!isRunning())
        return;

    if (m_source && m_source->bytesAvailable() > 0) {
        qint64 chunkSize = std::min(
                    m_source->bytesAvailable(),
                    maxBufferSize - m_buffer.size());
        if (chunkSize > 0) {
            qint64 orig_buffer_size = m_buffer.size();
            m_buffer.resize(orig_buffer_size + chunkSize);
            qint64 bytes = m_source->read(
                        m_buffer.data() + orig_buffer_size, chunkSize);
            if (bytes < 0) {
                // read error
                abort(tr("Read error: %1").arg(m_source->errorString()));
                return;
            }
            m_buffer.resize(orig_buffer_size + bytes);
            qCDebug(logger)
                    << "Read" << bytes
                    << "bytes. Buffer size:" << m_buffer.size();

            if (!m_source->isSequential() && m_source->atEnd()) {
                // random access file that is at end of file
                m_sourceEof = true;
            }

            if (m_size >= 0 && m_writtenBytes + m_buffer.size() > m_size) {
                return abort(tr("Read more bytes of input stream than "
                                "expected."));
            }
        }
    }

    if (m_destination && m_buffer.size() > 0) {
        qint64 bytes = m_destination->write(m_buffer);
        if (bytes < 0) {
            // write error
            abort(tr("Write error: %1").arg(m_destination->errorString()));
            return;
        }
        m_buffer.remove(0, bytes);
        m_writtenBytes += bytes;

        if (m_destination->bytesToWrite() == 0) {
            m_flushedBytes = m_writtenBytes;
        }

        qCDebug(logger)
                << "Written" << bytes
                << "bytes. Buffer size:" << m_buffer.size()
                << "Waiting for" << m_destination->bytesToWrite() << "bytes";
    }

    if (m_source
            && m_source->bytesAvailable() > 0
            && m_buffer.size() != maxBufferSize) {
        QTimer::singleShot(0, this, &CopyJob::poll);
    }

    if (m_sourceEof && m_writtenBytes == m_flushedBytes) {
        qCDebug(logger) << "EOF";

        // success
        exit();
        return;
    }
}

void CopyJob::pollAtSourceClose()
{
    if (!isRunning() || !m_source)
        return;

    qCDebug(logger) << "Detected source closing";

    m_sourceEof = true;

    qint64 readBytes = m_writtenBytes + m_buffer.size();
    if (m_source) readBytes += m_source->bytesAvailable();

    if (m_size >= 0 && readBytes < m_size) {
        return abort(tr("Early end of input stream"));
    }

    poll();
}

void CopyJob::pollAtDestinationClose()
{
    if (!isRunning() || !m_destination)
        return;

    qCDebug(logger) << "Detected destination closing";
    if (isRunning()) {
        // poll() exits/aborts or it will be aborted here
        abort(tr("Early end of output stream"));
    }
}

void CopyJob::pollBytesWritten(qint64 bytes)
{
    if (!isRunning())
        return;


    m_flushedBytes += bytes;
    setProcessedBytes(m_flushedBytes);

    qCDebug(logger) << "Send" << bytes << "Bytes ->"
                   << m_flushedBytes << "/" << m_size;

    poll();
}

} // namespace SailfishConnect
