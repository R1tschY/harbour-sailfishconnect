/**
 * Copyright 2013 Albert Vaca <albertvaka@gmail.com>
 * Copyright 2020 Richard Liebscher <richard.liebscher@gmail.com>.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "shareplugin.h"

#include <QLoggingCategory>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QFile>
#include <QUrl>
#include <QDateTime>
#include <QTemporaryFile>
#include <KJob>
#include <KPluginFactory>
#include <KJobTrackerInterface>
#include <KLocalizedString>

#include <core/daemon.h>

#include <downloadjob.h>
#include <filehelper.h>

using namespace SailfishConnect;


K_PLUGIN_CLASS_WITH_JSON(SharePlugin, "sailfishconnect_share.json")

static Q_LOGGING_CATEGORY(logger, "SailfishConnect.Share")

static const QString PACKET_TYPE_SHARE_REQUEST = 
    QStringLiteral("kdeconnect.share.request");
static const QString PACKET_TYPE_SHARE_REQUEST_UPDATE = 
    QStringLiteral("kdeconnect.share.request.update");

static void setDateModified(const QString& destination, const qint64 timestamp)
{
    // TODO: Qt 5.10
    // QFile receivedFile(destination);
    // bool succcess = receivedFile.setFileTime(
    //     QDateTime::fromMSecsSinceEpoch(timestamp), 
    //     QFileDevice::FileModificationTime);
    // if (!succcess) {
    //     qCWarning(logger).noquote().nospace()
    //             << "Failed to set modified file time for "
    //             << destination << ": "
    //             << receivedFile.errorString();
    // }
}

QString SharePlugin::incomingPath() const
{
    const QString downloadPath = 
        QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    const QString dir = config()->get<QString>(
        QStringLiteral("incoming_path"), downloadPath);

    if (dir.contains(QLatin1String("%1"))) {
        return dir.arg(device()->name());
    }

    if (QDir(dir).isRelative()) {
        return QDir(downloadPath).absoluteFilePath(dir);
    } else {
        return dir;
    }
}

bool SharePlugin::receivePacket(const NetworkPacket& np)
{
    if (np.hasPayload()) {
        const QString filename = escapeForFilePath(np.get<QString>(
            QStringLiteral("filename"),
            device()->name()));
        qint64 dateModified = np.get<qint64>(
            QStringLiteral("lastModified"), QDateTime::currentMSecsSinceEpoch());
        const bool open = np.get<bool>(QStringLiteral("open"), false);
        qDebug(logger) << "Start receiving file" << filename << dateModified << open;

        KJob* job = new DownloadJob(
            device()->id(), np.payload(), incomingPath() % "/" % filename, np.payloadSize(), this);
        Daemon::instance()->jobTracker()->registerJob(job);
        connect(job, &KJob::result, 
            this, [=](KJob* job) { finished(job, dateModified, open); });
        job->start();
    } else if (np.has(QStringLiteral("text"))) {
        QString text = np.get<QString>(QStringLiteral("text"));
        qDebug(logger) << "Received Text" << text;

        QTemporaryFile tmpFile;
        tmpFile.setFileTemplate(QStringLiteral("kdeconnect-XXXXXX.txt"));
        tmpFile.setAutoRemove(false);

        if (!tmpFile.open()) {
            qCWarning(logger)
                << "Share failed: failed to create temporary text file"
                << tmpFile.fileName();
            return true;
        }

        tmpFile.write(text.toUtf8());
        tmpFile.close();

        const QString fileName = tmpFile.fileName();
        Q_EMIT shareReceived(fileName);
        QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
    } else if (np.has(QStringLiteral("url"))) {
        QUrl url = QUrl::fromEncoded(np.get<QByteArray>(QStringLiteral("url")));
        qDebug(logger) << "Received URL" << url;
        QDesktopServices::openUrl(url);
        Q_EMIT shareReceived(url);
    } else {
        qCWarning(logger) << "Empty share received";
    }

    return true;
}

void SharePlugin::shareUrl(const QUrl& url, bool open)
{
    NetworkPacket packet(PACKET_TYPE_SHARE_REQUEST);
    if (url.isLocalFile()) {
        QSharedPointer<QFile> ioFile(new QFile(url.toLocalFile()));

        if (!ioFile->exists()) {
            Daemon::instance()->reportError(i18n("Could not share file"), i18n("%1 does not exist", url.toLocalFile()));
            return;
        } else {
            packet.setPayload(ioFile, ioFile->size());
            packet.set<QString>(QStringLiteral("filename"), QUrl(url).fileName());
            packet.set<bool>(QStringLiteral("open"), open);
        }
    } else {
        packet.set<QString>(QStringLiteral("url"), url.toString());
    }
    sendPacket(packet);
}

void SharePlugin::finished(KJob* job, qint64 dateModified, bool open)
{
    DownloadJob* download = qobject_cast<DownloadJob*>(job);
    if (!job->error()) {
        Q_EMIT shareReceived(download->destination());
        setDateModified(download->destination(), dateModified);
        qCDebug(logger)
            << "Share succeeded:" << download->destination();
        if (open) {
            QDesktopServices::openUrl(download->destination());
        }
    } else {
        qCWarning(logger)
            << "Share failed:"
            << job->errorText()
            ; //<< job->target().toString();
    }
}

void SharePlugin::shareUrls(const QStringList& urls) {
    for(const QString& url : urls) {
        shareUrl(QUrl(url), false);
    }
}

void SharePlugin::shareText(const QString& text)
{
    NetworkPacket packet(PACKET_TYPE_SHARE_REQUEST);
    packet.set<QString>(QStringLiteral("text"), text);
    sendPacket(packet);
}

QString SharePlugin::dbusPath() const
{
    return QStringLiteral("/modules/kdeconnect/devices/") + device()->id() + QStringLiteral("/share");
}

#include "shareplugin.moc"