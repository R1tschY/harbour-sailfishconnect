#include "copyjob.h"

#include <QIODevice>
#include <QLoggingCategory>

namespace SailfishConnect {

static Q_LOGGING_CATEGORY(logger, "sailfishconnect.io")

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

    if (m_source)
        m_source->close();

    if (m_destination)
        m_destination->close();
}

void CopyJob::setDestination(const QSharedPointer<QIODevice> &destination)
{
    if (!isPending())
        return;

    m_destination = destination;
}

void CopyJob::setSource(const QSharedPointer<QIODevice> &source)
{
    if (!isPending())
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

    setTotalBytes(
                (m_size >= 0)
                ? m_size
                : (m_source->isSequential()) ? m_source->size() : -1);


    connect(m_source.data(), &QIODevice::readChannelFinished,
            this, &CopyJob::pollAtEof);
    connect(m_source.data(), &QIODevice::readyRead,
            this, &CopyJob::poll);
    connect(m_destination.data(), &QIODevice::bytesWritten,
            this, &CopyJob::poll);
    connect(m_destination.data(), &QIODevice::aboutToClose,
            this, &CopyJob::poll);

    poll();
}

void CopyJob::poll()
{
    if (!isRunning())
        return;

    if (m_source->bytesAvailable() > 0) {
        qint64 orig_buffer_size = m_buffer.size();
        m_buffer.resize(orig_buffer_size + m_source->bytesAvailable());
        qint64 bytes = m_source->read(m_buffer.data(), m_buffer.size());
        if (bytes < 0) {
            // read error
            abort(tr("Read error: %1").arg(m_source->errorString()));
            return;
        }
        Q_ASSERT(m_source->bytesAvailable() == 0);
        m_buffer.resize(orig_buffer_size + bytes);
        qCDebug(logger)
                << "Read" << bytes
                << "bytes. Buffer size:" << m_buffer.size();
    }

    if (m_buffer.size() > 0) {
        qint64 bytes = m_destination->write(m_buffer.data(), m_buffer.size());
        if (bytes < 0) {
            // write error
            abort(tr("Write error: %1").arg(m_source->errorString()));
            return;
        }
        m_buffer.remove(0, bytes);

        qCDebug(logger)
                << "Written" << bytes
                << "bytes. Buffer size:" << m_buffer.size();

        m_writtenBytes += bytes;
        setProcessedBytes(m_writtenBytes);
    }

    if (m_size >= 0 && m_writtenBytes + m_buffer.size() > m_size) {
        return abort(tr("Read more bytes of input stream than expected."));
    }


    if (m_eof && m_buffer.isEmpty()) {
        qCDebug(logger) << "EOF";

        // success
        exit();
        return;
    }
}

void CopyJob::pollAtEof() {
    qCDebug(logger) << "Detected EOF";

    m_eof = true;

    if (m_size >= 0 && m_writtenBytes + m_buffer.size() < m_size) {
        return abort(tr("Early end of input stream"));
    }

    poll();
}

} // namespace SailfishConnect
