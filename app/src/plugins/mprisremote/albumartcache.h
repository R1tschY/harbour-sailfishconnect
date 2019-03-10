#ifndef ALBUMARTCACHE_H
#define ALBUMARTCACHE_H

#include <QObject>
#include <QQuickAsyncImageProvider>
#include <QCache>
#include <QImage>
#include <QSet>
#include <QDir>

class KdeConnectConfig;

namespace SailfishConnect {

class Job;

class AlbumArtCache : public QObject
{
    Q_OBJECT
public:
    explicit AlbumArtCache(
            KdeConnectConfig* config,
            const QString &deviceId, QObject *parent = nullptr);

    void init();

    bool startFetching(const QUrl& url);
    Job* endFetching(const QUrl& url, const QSharedPointer<QIODevice>& payload);

    Job* getFetchingJob(const QString& hash);

    /**
     * @brief Get album art what is yet available.
     *
     * Album art is not fetch from remote or internet.
     *
     * @param url remote album art url
     * @return album art or null image, when not in cache.
     */
    QImage getAvailable(const QUrl& url);

    QImage getCacheFile(const QString& hashFile);

    static QString hashFor(const QUrl& url);
    QString cacheFileFor(const QUrl& url) const;
    QString cacheFileNameFor(const QUrl& url) const;

    QUrl imageUrl(const QUrl& url) const;

signals:

public slots:

private:
    KdeConnectConfig* m_config;
    QString m_deviceId;

    QHash<QString, Job*> m_fetching;
    QSet<QString> m_diskCache;
    qint64 m_diskCacheSize = 0;
    QDir m_cacheDir;

    void fetchFinished();
};


class AlbumArtProvider : public QQuickAsyncImageProvider
{
public:
    AlbumArtProvider();

    QQuickImageResponse *requestImageResponse(
            const QString &id, const QSize &requestedSize) override;
};


class AlbumArtImageResponse : public QQuickImageResponse {
    Q_OBJECT
public:
    AlbumArtImageResponse(
            AlbumArtCache* cache, Job* job);

    QQuickTextureFactory *textureFactory() const override;
    QString errorString() const override;

public slots:
    void cancel() override;

private:
    AlbumArtCache* m_cache;
    Job* m_job;
};


class CachedAlbumArtImageResponse : public QQuickImageResponse {
public:
    CachedAlbumArtImageResponse(const QImage& image);

    QQuickTextureFactory *textureFactory() const override;

private:
    QImage m_image;
};

} // namespace SailfishConnect

#endif // ALBUMARTCACHE_H
