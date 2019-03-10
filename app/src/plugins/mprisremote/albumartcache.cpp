#include "albumartcache.h"

#include <QQuickTextureFactory>
#include <QCryptographicHash>
#include <QFile>
#include <QDir>
#include <QSharedPointer>
#include <QLoggingCategory>
#include <QFileInfo>

#include <sailfishconnect/io/job.h>
#include <sailfishconnect/io/copyjob.h>
#include <sailfishconnect/kdeconnectconfig.h>
#include <sailfishconnect/helper/humanize.h>
#include <sailfishconnect/daemon.h>
#include <sailfishconnect/device.h>

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
        qCCritical(logger) << "Failed to create cache dir" << m_cacheDir;
        return;
    }

    const QFileInfoList files = m_cacheDir.entryInfoList(
                QDir::Files | QDir::NoDotAndDotDot);
    for (const auto& file : files) {
        m_diskCache.insert(file.baseName());
        m_diskCacheSize += file.size();
    }
    qCInfo(logger) << "Using" << humanizeBytes(m_diskCacheSize)
                   << "of album art cache";
}

Job *AlbumArtCache::endFetching(
        const QUrl &url, const QSharedPointer<QIODevice> &payload)
{
    QString hash = hashFor(url);
    Job* job = m_fetching.value(hash);
    if (job != nullptr)
        return job;

    QSharedPointer<QFile> file { new QFile(cacheFileFor(url)) };

    if (!file->open(QIODevice::WriteOnly | QIODevice::Unbuffered)) {
        qCCritical(logger)
                << "Failed to create cache file" << file->fileName()
                << file->errorString();
        return nullptr;
    }

    // TODO: support cancelation, copy from DownloadJob
    job = new CopyJob(m_deviceId, payload, file, -1, this);
    m_fetching.insert(hash, job);
    connect(job, &Job::finished, this, &AlbumArtCache::fetchFinished);
    job->start();
    job->setObjectName(url.toString());
    return job;
}

Job *AlbumArtCache::getFetchingJob(const QString &hash)
{
    return m_fetching.value(hash, nullptr);  // TODO: broken
}

bool AlbumArtCache::startFetching(const QUrl &url)
{
    if (url.isEmpty())
        return false;

    QString hash = hashFor(url);
    if (m_diskCache.contains(hash) || m_fetching.contains(hash)) {
        qCDebug(logger) << url << "already cached";
        return false;
    }

    m_fetching.insert(hash, nullptr);
    return true;
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
    return QUrl(QStringLiteral("image:/albumart/%1/%2").arg(
                    m_deviceId, cacheFileNameFor(url)));
}

void AlbumArtCache::fetchFinished()
{
    Job* sender = qobject_cast<Job*>(QObject::sender());
    Q_ASSERT(sender != nullptr);
    QUrl url = sender->objectName();
    QString hash = m_fetching.key(sender);  // TODO: add url prop to job
    m_fetching.remove(hash);
    m_diskCache.insert(hash);

    if (!sender->errorString().isEmpty()) {
        qCDebug(logger) << "Failed download of" << url;

        // mark as failed
        QFile file { cacheFileFor(url) };
        file.open(QIODevice::WriteOnly);
        file.close();
    } else {
        m_diskCacheSize += sender->processedBytes();
        qCDebug(logger).nospace()
                << "Added " << url
                << " (Disk cache: " << humanizeBytes(m_diskCacheSize) << ")";
    }
}

// -----------------------------------------------------------------------------

AlbumArtProvider::AlbumArtProvider()
{

}

QQuickImageResponse *AlbumArtProvider::requestImageResponse(
        const QString &id, const QSize &requestedSize)
{
    auto parts = id.split(QChar('/'));
    if (parts.length() != 2) {
        return new CachedAlbumArtImageResponse(QImage());
    }

    Device* device = Daemon::instance()->getDevice(parts[0]);
    if (device == nullptr) {
        return new CachedAlbumArtImageResponse(QImage());
    }

    MprisRemotePlugin* plugin = qobject_cast<MprisRemotePlugin*>(
                device->plugin("SailfishConnect::MprisRemotePlugin"));
    if (plugin == nullptr) {
        return new CachedAlbumArtImageResponse(QImage());
    }

    QFileInfo fileName { parts[1] };
    AlbumArtCache* cache = plugin->albumArtCache();
    QString hash = fileName.baseName();
    Job* job = cache->getFetchingJob(hash);
    if (job) {
        return new AlbumArtImageResponse(cache, job);
    }

    return new CachedAlbumArtImageResponse(cache->getCacheFile(parts[1]));
}

// -----------------------------------------------------------------------------

AlbumArtImageResponse::AlbumArtImageResponse(AlbumArtCache *cache, Job *job)
    : m_cache(cache)
    , m_job(job)
{
    if (job->isFinished()) {
        QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection);
        return;
    }
    connect(
        job, &Job::finished, this, &AlbumArtImageResponse::finished,
        Qt::QueuedConnection);
}

QQuickTextureFactory *AlbumArtImageResponse::textureFactory() const
{
    return QQuickTextureFactory::textureFactoryForImage(
                m_cache->getAvailable(m_job->target()));
}

QString AlbumArtImageResponse::errorString() const
{
    return m_job->errorString();
}

void AlbumArtImageResponse::cancel()
{
    m_job->cancel();
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

} // namespace SailfishConnect
