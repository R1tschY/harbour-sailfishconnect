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

#ifndef ALBUMARTCACHE_H
#define ALBUMARTCACHE_H

#include <QObject>
#include <QQuickAsyncImageProvider>
#include <QCache>
#include <QImage>
#include <QSet>
#include <QDir>
#include <KJob>

class KdeConnectPluginConfig;
class QQmlEngine;
class KJob;

class DownloadAlbumArtJob : public QObject
{
    Q_OBJECT
public:
    DownloadAlbumArtJob(const QUrl& url, const QString& filePath, QObject* parent = nullptr);

    bool gotData(const QSharedPointer<QIODevice> &payload);

    QUrl url() const { return m_url; }
    QString hash() const { return m_hash; }
    QString filePath() const { return m_filePath; }
    QString fileName() const;
    qlonglong fileSize() const { return m_fileSize; }
    bool isFetching() const { return m_fileTransfer != nullptr; }
    KJob *fileTransfer() const;

signals:
    void finished(const QString& cacheFile, const QString& errorString);

private:
    QUrl m_url;
    QString m_hash;
    QString m_filePath;
    qlonglong m_fileSize;
    KJob* m_fileTransfer = nullptr;
    int m_redirectCount = 0;

    void failed(const QString& error);
    void fetchFinished(KJob* fileTransfer);
};

class AlbumArtCache : public QObject
{
    Q_OBJECT
public:
    explicit AlbumArtCache(
        const QString& deviceId, KdeConnectPluginConfig *config, QObject* parent);

    void init();

    DownloadAlbumArtJob* startFetching(const QUrl& url);
    void endFetching(
            const QUrl& url, const QSharedPointer<QIODevice>& payload);

    DownloadAlbumArtJob* getFetchingJob(const QString& hash);

    bool isAvailable(const QUrl& url) const;
    bool isHashAvailable(const QString& hash) const;

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
    KdeConnectPluginConfig* m_config;
    QString m_deviceId;

    QHash<QString, DownloadAlbumArtJob*> m_fetching;
    QHash<QString, QString> m_diskCache;
    qint64 m_diskCacheSize = 0; // TODO: use QCache for cache handling
    QDir m_cacheDir;

    void fetchFinished(const QString &cacheFile, const QString &errorString);
};


class AlbumArtProvider : public QObject, public QQuickAsyncImageProvider
{
    Q_OBJECT
public:
    QQuickImageResponse *requestImageResponse(
            const QString &id, const QSize &requestedSize) override;

    static void registerImageProvider(QQmlEngine* qmlEngine);

private slots:
    QQuickImageResponse* unsafeRequestImageResponse(
            QString id, QSize requestedSize, QThread *targetThread);

private:
    QQuickImageResponse* unsafeRequestImageResponse_(
            const QString &id, const QSize &requestedSize);
};


class AlbumArtImageResponse : public QQuickImageResponse {
    Q_OBJECT
public:
    AlbumArtImageResponse(DownloadAlbumArtJob* job);

    QQuickTextureFactory *textureFactory() const override;
    QString errorString() const override;

private:
    QUrl m_url;
    QString m_errorString;
    QImage m_image;

    void onFinished(const QString &cacheFile, const QString &errorString);
    void onJobDestroyed();
};


class CachedAlbumArtImageResponse : public QQuickImageResponse {
public:
    CachedAlbumArtImageResponse(const QImage& image);

    QQuickTextureFactory *textureFactory() const override;

private:
    QImage m_image;
};


#endif // ALBUMARTCACHE_H
