#pragma once

#include <array>

#include <QSharedPointer>
#include <QString>
#include <QTimer>

#include "job.h"

class QIODevice;

namespace SailfishConnect {

class CopyJob : public Job
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
            QSharedPointer<QIODevice> source,
            QSharedPointer<QIODevice> destination,
            qint64 size = -1,
            QObject *parent = nullptr);

    QSharedPointer<QIODevice> source() const { return m_source; }
    QSharedPointer<QIODevice> destination() const { return m_destination; }
    void setSource(const QSharedPointer<QIODevice> &source);
    void setDestination(const QSharedPointer<QIODevice> &destination);

protected:
    void close();
    void doStart() override;

private:
    QSharedPointer<QIODevice> m_source;
    QSharedPointer<QIODevice> m_destination;

    qint64 m_size = -1;
    qint64 m_writtenBytes = 0;
    bool m_sourceEof = false;
    bool m_started = false;
    QTimer m_timer;

    std::size_t m_bufferSize = 0;
    std::array<char, 64 * 1024> m_buffer;

    void pollAtSourceClose();
    void pollAtDestinationClose();

    void closeSource();
    void closeDestination();

    void checkSource();
    void checkDestination();

    void finish();

private slots:
    void poll();
};

} // namespace SailfishConnect
