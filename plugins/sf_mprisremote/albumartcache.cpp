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

#include "albumartcache.h"

#include <QQuickTextureFactory>
#include <QCryptographicHash>
#include <QFile>
#include <QDir>
#include <QSharedPointer>
#include <QLoggingCategory>
#include <QFileInfo>
#include <QQmlEngine>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QStandardPaths>
#include <QThread>

#include <copyjob.h>
#include <core/kdeconnectconfig.h>
#include <core/daemon.h>
#include <core/device.h>

#include "mprisremoteplugin.h"

using namespace SailfishConnect;

static Q_LOGGING_CATEGORY(logger, "kdeconnect.plugin.mprisremote.albumartcache")

static QString getCacheDir(const QString& id) {
    return QStandardPaths::writableLocation(QStandardPaths::CacheLocation) % QChar::fromLatin1('/') % id % QStringLiteral("/albumart");
}

AlbumArtCache::AlbumArtCache(const QString& deviceId, KdeConnectPluginConfig *config, QObject* parent)
    : QObject(parent)
    , m_config(config)
    , m_deviceId(deviceId)
{
    m_cacheDir = QDir(getCacheDir(deviceId));
    if (!m_cacheDir.mkpath(QStringLiteral("."))) {
        qCCritical(logger).noquote()
                << "Failed to create cache dir" << m_cacheDir;
        return;
    }

    const QFileInfoList files = m_cacheDir.entryInfoList(
                QDir::Files | QDir::NoDotAndDotDot);
    for (const auto& file : files) {
        m_diskCache.insert(file.baseName(), file.fileName());
        m_diskCacheSize += file.size();
    }
    qCInfo(logger).noquote()
            << "Using" << (m_diskCacheSize / 1024 / 1024)
            << "MB of album art cache";
}

DownloadAlbumArtJob *AlbumArtCache::getFetchingJob(const QString &hash)
{
    return m_fetching.value(hash, nullptr);
}

bool AlbumArtCache::isAvailable(const QUrl& url) const
{
    return m_diskCache.contains(hashFor(url));
}

bool AlbumArtCache::isHashAvailable(const QString &hash) const
{
    return m_diskCache.contains(hash);
}

QImage AlbumArtCache::getAvailable(const QUrl &url)
{
    return QImage(cacheFileFor(url));
}

QImage AlbumArtCache::getCacheFile(const QString &hashFile)
{
    return QImage(m_cacheDir.filePath(hashFile));
}

QString AlbumArtCache::hashFor(const QUrl &url)
{
    return QString::fromLatin1(
                QCryptographicHash::hash(
                    url.toEncoded(), QCryptographicHash::Md5).toHex());
}

QString AlbumArtCache::cacheFileFor(const QUrl& url) const
{
    auto fileExt = QFileInfo(url.fileName()).suffix();
    if (fileExt.isEmpty()) {
        return m_cacheDir.path() % QChar::fromLatin1('/') % hashFor(url);
    } else {
        return m_cacheDir.path() % QChar::fromLatin1('/') % hashFor(url) % QChar::fromLatin1('.') % fileExt;
    }
}

QString AlbumArtCache::cacheFileNameFor(const QUrl &url) const
{
    auto fileExt = QFileInfo(url.fileName()).suffix();
    if (fileExt.isEmpty()) {
        return hashFor(url);
    } else {
        return hashFor(url) % QChar::fromLatin1('.') % fileExt;
    }
}

QUrl AlbumArtCache::imageUrl(const QUrl &url) const
{
    return QUrl(QStringLiteral("image://albumart/%1/%2").arg(
                    m_deviceId, cacheFileNameFor(url)));
}

DownloadAlbumArtJob *AlbumArtCache::startFetching(const QString& originalUrl, const QString& playerName)
{
    if (originalUrl.isEmpty())
        return nullptr;

    QUrl url{originalUrl};

    QString hash = hashFor(url);
    if (m_diskCache.contains(hash) || m_fetching.contains(hash)) {
        qCDebug(logger) << originalUrl << "already cached";
        return nullptr;
    }

    auto* job = new DownloadAlbumArtJob(url, cacheFileFor(url), this);
    // TODO: add timeout to remove it after a time when no response
    m_fetching.insert(hash, job);
    connect(job, &DownloadAlbumArtJob::finished,
            this, &AlbumArtCache::fetchFinished);

    if (!url.isLocalFile()) {
        // only request urls starting with file://. HTTP requests can we do on our own.
        auto network = Daemon::instance()->networkAccessManager();
        job->gotData(QSharedPointer<QIODevice>(network->get(QNetworkRequest(url))), -1);
    } else {
        Q_EMIT requestAlbumArt(originalUrl, playerName);
    }

    return job;
}

void AlbumArtCache::endFetching(
        const QUrl &url, const QSharedPointer<QIODevice>& payload, qint64 payloadSize)
{
    DownloadAlbumArtJob* job = m_fetching.value(hashFor(url));
    if (job == nullptr) {
        qCDebug(logger) << "Never started a job for" << url;
        return;
    }

    job->gotData(payload, payloadSize);
}

void AlbumArtCache::fetchFinished(
        const QString& cacheFile, const QString& errorString)
{
    auto* job = qobject_cast<DownloadAlbumArtJob*>(sender());
    Q_ASSERT(job != nullptr);

    m_fetching.remove(job->hash());

    if (errorString.isEmpty()) {
        m_diskCache.insert(job->hash(), job->fileName());
        m_diskCacheSize += job->fileSize();

        qCDebug(logger).nospace()
                << "Added " << job->url()
                << " (Disk cache: " << (m_diskCacheSize / 1014 / 1024) << "MB )";
    }
}

// -----------------------------------------------------------------------------

void AlbumArtProvider::registerImageProvider(QQmlEngine* qmlEngine) {
    qmlEngine->addImageProvider(
                QStringLiteral("albumart"), new AlbumArtProvider());
}

QQuickImageResponse *AlbumArtProvider::unsafeRequestImageResponse(
        QString id, QSize requestedSize, QThread *targetThread)
{
    QQuickImageResponse* result = unsafeRequestImageResponse_(
                id, requestedSize);
    result->moveToThread(targetThread);
    return result;
}

QQuickImageResponse *AlbumArtProvider::unsafeRequestImageResponse_(
        const QString& id, const QSize& requestedSize)
{
    auto parts = id.split(QChar::fromLatin1('/'));
    if (parts.length() != 2) {
        return new CachedAlbumArtImageResponse(QImage());
    }

    Device* device = Daemon::instance()->getDevice(parts[0]);
    if (device == nullptr) {
        qCDebug(logger) << "non-existing device" << parts[0];
        return new CachedAlbumArtImageResponse(QImage());
    }

    MprisRemotePlugin* plugin = qobject_cast<MprisRemotePlugin*>(
                device->plugin(QStringLiteral("sailfishconnect_mprisremote")));
    if (plugin == nullptr) {
        qCDebug(logger) << "mpris plugin not loaded";
        return new CachedAlbumArtImageResponse(QImage());
    }

    QFileInfo fileName { parts[1] };
    AlbumArtCache* cache = plugin->albumArtCache();
    QString hash = fileName.baseName();
    DownloadAlbumArtJob* job = cache->getFetchingJob(hash);
    if (job) {
        return new AlbumArtImageResponse(job);
    }

    if (!cache->isHashAvailable(hash)) {
        qCWarning(logger) << "image not cached yet:" << parts[1];
        return new CachedAlbumArtImageResponse(QImage());
    }

    // TODO: load image in other thread
    return new CachedAlbumArtImageResponse(cache->getCacheFile(parts[1]));
}

QQuickImageResponse *AlbumArtProvider::requestImageResponse(
        const QString &id, const QSize &requestedSize)
{
    QQuickImageResponse* result = nullptr;
    bool success = QMetaObject::invokeMethod(
                this, "unsafeRequestImageResponse",
                Qt::BlockingQueuedConnection,
                Q_RETURN_ARG(QQuickImageResponse*, result),
                Q_ARG(QString, id),
                Q_ARG(QSize, requestedSize),
                Q_ARG(QThread*, QThread::currentThread()));
    return success && result
            ? result : new CachedAlbumArtImageResponse(QImage());
}

// -----------------------------------------------------------------------------

AlbumArtImageResponse::AlbumArtImageResponse(DownloadAlbumArtJob *job)
    : m_url(job->url())
{
    connect(
        job, &DownloadAlbumArtJob::finished,
        this, &AlbumArtImageResponse::onFinished,
        Qt::QueuedConnection);
    connect(
        job, &DownloadAlbumArtJob::destroyed,
        this, &AlbumArtImageResponse::onJobDestroyed,
        Qt::QueuedConnection);
}

QQuickTextureFactory *AlbumArtImageResponse::textureFactory() const
{
    qCDebug(logger) << "Deliver" << m_url << m_image;
    return QQuickTextureFactory::textureFactoryForImage(m_image);
}

QString AlbumArtImageResponse::errorString() const
{
    return m_errorString;
}

void AlbumArtImageResponse::onFinished(
        const QString& cacheFile, const QString& errorString)
{
    m_errorString = errorString;
    if (errorString.isEmpty()) {
        m_image = QImage(cacheFile);
    }

    Q_EMIT finished();
}

void AlbumArtImageResponse::onJobDestroyed()
{
    m_errorString = QStringLiteral("job destroyed");
    Q_EMIT finished();
}

// -----------------------------------------------------------------------------

CachedAlbumArtImageResponse::CachedAlbumArtImageResponse(const QImage &image)
    : m_image(image)
{
    QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection);
}

QQuickTextureFactory *CachedAlbumArtImageResponse::textureFactory() const
{
    return QQuickTextureFactory::textureFactoryForImage(m_image);
}

// -----------------------------------------------------------------------------

DownloadAlbumArtJob::DownloadAlbumArtJob(
        const QUrl &url, const QString &filePath, QObject *parent)
    : QObject(parent)
    , m_url(url)
    , m_hash(AlbumArtCache::hashFor(url))
    , m_filePath(filePath)
{ }

bool DownloadAlbumArtJob::gotData(const QSharedPointer<QIODevice>& payload, qint64 payloadSize)
{
    if (isFetching()) {
        qCDebug(logger) << "Already downloading" << m_url;
        return false;
    }

    if (payload.isNull()) {
        qCDebug(logger) << "Empty payload";
        Q_EMIT finished(m_filePath, QStringLiteral("Empty payload"));
        return false;
    }

    QSharedPointer<QFile> file { new QFile(m_filePath) };
    if (!file->open(QIODevice::WriteOnly | QIODevice::Unbuffered)) {
        qCCritical(logger).noquote()
                << "Failed to create cache file" << file->fileName()
                << file->errorString();
        Q_EMIT finished(
            m_filePath, QStringLiteral("Failed to create cache file"));
        return false;
    }

    // TODO: support cancelation, copy from DownloadJob
    m_fileTransfer = new CopyJob(QString(), payload, file, payloadSize, this);
    connect(m_fileTransfer, &KJob::result,
            this, &DownloadAlbumArtJob::fetchFinished);
    m_fileTransfer->start();

    return true;
}

void DownloadAlbumArtJob::fetchFinished(KJob* fileTransfer)
{
    if (fileTransfer != m_fileTransfer)
        return;

    if (fileTransfer->error()) {
        failed(fileTransfer->errorString());
    } else {
        // TODO: make it prettier :D
        auto* fileTransferJob = qobject_cast<CopyJob*>(m_fileTransfer);
        if (fileTransferJob) {
            auto* reply = qobject_cast<QNetworkReply*>(
                        fileTransferJob->source().data());
            if (reply) {
                // TODO: in Qt 5.9 use QNetworkRequest::UserVerifiedRedirectPolicy
                QString location = reply->attribute(
                    QNetworkRequest::RedirectionTargetAttribute).toString();
                if (!location.isEmpty()) {
                    m_redirectCount += 1;

                    if (m_redirectCount > 10) {
                        failed(QStringLiteral("too many redirects"));
                        return;
                    }

                    auto network = Daemon::instance()->networkAccessManager();
                    // TODO: set QNetworkRequest::BackgroundRequestAttribute
                    m_fileTransfer = nullptr;
                    gotData(QSharedPointer<QIODevice>(
                        network->get(QNetworkRequest(QUrl(location)))), -1);
                    return;
                }

                int status = reply->attribute(
                    QNetworkRequest::HttpStatusCodeAttribute).toInt();
                if (status != 200) {
                    failed(QStringLiteral("status code was not 200"));
                    return;
                }
            }
        }

        m_fileSize = fileTransfer->processedAmount(KJob::Bytes);
        Q_EMIT finished(m_filePath, QString());
    }
}

void DownloadAlbumArtJob::failed(const QString &error)
{
    qCWarning(logger) << "Failed download of" << m_url.toString()
                      << error;

    // mark as failed
    QFile file { m_filePath };
    file.open(QIODevice::WriteOnly);
    file.close();

    Q_EMIT finished(m_filePath, error);
}

QString DownloadAlbumArtJob::fileName() const
{
    return QFileInfo(m_filePath).fileName();
}

KJob *DownloadAlbumArtJob::fileTransfer() const
{
    return m_fileTransfer;
}
