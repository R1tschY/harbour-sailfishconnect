#include "albumartcache.h"

#include <QQuickTextureFactory>
#include <QCryptographicHash>
#include <QFile>
#include <QDir>
#include <QSharedPointer>
#include <QLoggingCategory>
#include <QFileInfo>
#include <QQmlEngine>

#include <sailfishconnect/io/copyjob.h>
#include <sailfishconnect/kdeconnectconfig.h>
#include <sailfishconnect/helper/humanize.h>
#include <sailfishconnect/daemon.h>
#include <sailfishconnect/device.h>
#include <appdaemon.h>

#include "mprisremoteplugin.h"

namespace SailfishConnect {

static Q_LOGGING_CATEGORY(logger, "kdeconnect.plugin.mprisremote.albumartcache")

AlbumArtCache::AlbumArtCache(
        KdeConnectConfig *config, const QString& deviceId, QObject *parent)
    : QObject(parent)
    , m_config(config)
    , m_deviceId(deviceId)
{
    // TODO: add api to SailfishConnectPluginConfig
    QDir pluginConfigDir = m_config->pluginConfigDir(
        m_deviceId, QStringLiteral("SailfishConnect::MprisRemotePlugin"));
    m_cacheDir = QDir(pluginConfigDir.filePath(QStringLiteral("albumart")));

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
            << "Using" << humanizeBytes(m_diskCacheSize)
            << "of album art cache";
}

DownloadAlbumArtJob *AlbumArtCache::endFetching(
        const QUrl &url, const QSharedPointer<QIODevice> &payload)
{
    QString hash = hashFor(url);
    DownloadAlbumArtJob* job = m_fetching.value(hash);
    if (job == nullptr) {
        qCDebug(logger) << "Never started a job for" << url;
        return nullptr;
    }
    if (job->isFetching()) {
        qCDebug(logger) << "Already downloading" << url;
        return job;
    }

    if (payload.isNull()) {
        qCDebug(logger) << "Empty payload";
        m_fetching.remove(hash);
        return nullptr;
    }

    QSharedPointer<QFile> file { new QFile(cacheFileFor(url)) };
    if (!file->open(QIODevice::WriteOnly | QIODevice::Unbuffered)) {
        qCCritical(logger).noquote()
                << "Failed to create cache file" << file->fileName()
                << file->errorString();
        m_fetching.remove(hash);
        return nullptr;
    }

    // TODO: support cancelation, copy from DownloadJob
    auto* fileTransfer = new CopyJob(m_deviceId, payload, file, -1, this);
    job->gotData(fileTransfer);
    return job;
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

DownloadAlbumArtJob *AlbumArtCache::startFetching(const QUrl &url)
{
    if (url.isEmpty())
        return nullptr;

    QString hash = hashFor(url);
    if (m_diskCache.contains(hash) || m_fetching.contains(hash)) {
        qCDebug(logger) << url << "already cached";
        return nullptr;
    }

    auto* job = new DownloadAlbumArtJob(url, cacheFileFor(url), this);
    // TODO: add timeout to remove it after a time when no response
    m_fetching.insert(hash, job);
    connect(job, &DownloadAlbumArtJob::finished,
            this, &AlbumArtCache::fetchFinished);
    return job;
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
    return m_cacheDir.path() % QChar('/') % hashFor(url) % QChar('.') % fileExt;
}

QString AlbumArtCache::cacheFileNameFor(const QUrl &url) const
{
    auto fileExt = QFileInfo(url.fileName()).suffix();
    return hashFor(url) % QChar('.') % fileExt;
}

QUrl AlbumArtCache::imageUrl(const QUrl &url) const
{
    return QUrl(QStringLiteral("image://albumart/%1/%2").arg(
                    m_deviceId, cacheFileNameFor(url)));
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
                << " (Disk cache: " << humanizeBytes(m_diskCacheSize) << ")";
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
    auto parts = id.split(QChar('/'));
    if (parts.length() != 2) {
        return new CachedAlbumArtImageResponse(QImage());
    }

    Device* device = Daemon::instance()->getDevice(parts[0]);
    if (device == nullptr) {
        qCDebug(logger) << "non-existing device" << parts[0];
        return new CachedAlbumArtImageResponse(QImage());
    }

    qCDebug(logger) << "plugins" << device->loadedPlugins();
    MprisRemotePlugin* plugin = qobject_cast<MprisRemotePlugin*>(
                device->plugin("SailfishConnect::MprisRemotePlugin"));
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

    emit finished();
}

void AlbumArtImageResponse::onJobDestroyed()
{
    m_errorString = QStringLiteral("job destroyed");
    emit finished();
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

void DownloadAlbumArtJob::gotData(KJob* fileTransfer)
{
    m_fileTransfer = fileTransfer;

    connect(fileTransfer, &KJob::result,
            this, &DownloadAlbumArtJob::fetchFinished);

    fileTransfer->start();
}

void DownloadAlbumArtJob::fetchFinished(KJob* fileTransfer)
{
    if (fileTransfer->error()) {
        qCDebug(logger) << "Failed download of" << m_url;

        // mark as failed
        QFile file { m_filePath };
        file.open(QIODevice::WriteOnly);
        file.close();

        finished(m_filePath, fileTransfer->errorString());
    } else {
        m_fileSize = fileTransfer->processedAmount(KJob::Bytes);
        finished(m_filePath, QString());
    }
}

QString DownloadAlbumArtJob::fileName() const
{
    return QFileInfo(m_filePath).fileName();
}

} // namespace SailfishConnect
