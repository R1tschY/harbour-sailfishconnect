/*
 * Copyright 2018 Richard Liebscher <richard.liebscher@gmail.com>.
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

#include "shareplugin.h"

#include <QGuiApplication>
#include <QClipboard>
#include <QLoggingCategory>
#include <QStandardPaths>
#include <QDateTime>
#include <QTemporaryFile>
#include <QDesktopServices>
#include <QtQml>

#include <sailfishconnect/helper/filehelper.h>
#include <sailfishconnect/io/jobmanager.h>
#include <sailfishconnect/daemon.h>
#include <sailfishconnect/device.h>
#include <sailfishconnect/kdeconnectpluginconfig.h>

namespace SailfishConnect {

static Q_LOGGING_CATEGORY(logger, "SailfishConnect.Share")

static QString packetType = QStringLiteral("kdeconnect.share.request");

SharePlugin::SharePlugin(
        Device* device,
        const QString &name,
        const QSet<QString> &outgoingCapabilities)
    : KdeConnectPlugin(device, name, outgoingCapabilities)
{ }

QString SharePlugin::incomingPath() const
{
    const QString downloadPath =
            QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    const QString dir = config()->get<QString>(
                QStringLiteral("incoming_path"), downloadPath);

    if (dir.contains(QLatin1String("%1"))) {
        return dir.arg(device()->name());
    }

    return dir;
}

bool SharePlugin::receivePacket(const NetworkPacket& np)
{
    if (np.hasPayload()) {
        const QString filename = escapeForFilePath(np.get<QString>(
                    QStringLiteral("filename"),
                    device()->name()));

        Job* job = np.createDownloadPayloadJob(
                    incomingPath() % "/" % filename);
        job->setParent(this);
        Daemon::instance()->jobManager()->addJob(job, device()->id());
        connect(job, &Job::finished, this, &SharePlugin::finishedFileTransfer);
        // TODO: daemon->addJob(job);
        // TODO: add to a job queue in which only x downloads/uploads are
        // running in parallel
        job->start();
    } else if (np.has(QStringLiteral("text"))) {
        auto text = np.get<QString>(QStringLiteral("text"));
        const QString filename = escapeForFilePath(device()->name());
        const QString filepath = QStringLiteral("%1/%2.txt").arg(
                    incomingPath(), filename);

        QFile textFile(nonexistingFile(filepath).filePath());
        if (!textFile.open(QIODevice::WriteOnly)) {
            qCWarning(logger)
                    << "Share failed: failed to create temporary text file"
                    << textFile.fileName();
            return true;
        }

        textFile.write(text.toUtf8());
        textFile.close();

        emit received(QUrl::fromLocalFile(textFile.fileName()));
    } else if (np.has(QStringLiteral("url"))) {
        QUrl url = QUrl::fromEncoded(np.get<QByteArray>(QStringLiteral("url")));
        QDesktopServices::openUrl(url);
        emit received(url);
    } else {
        qCWarning(logger) << "Empty share received";
    }

    return true;
}

void SharePlugin::share(const QUrl& url)
{
    NetworkPacket packet(packetType);
    if (url.isLocalFile()) {
        QSharedPointer<QIODevice> localFile(new QFile(url.toLocalFile()));
        packet.setPayload(localFile, localFile->size());
        packet.set<QString>(QStringLiteral("filename"), url.fileName());
        sendPacket(packet, Daemon::instance()->jobManager());
    } else {
        packet.set<QString>(QStringLiteral("url"), url.toString());
        sendPacket(packet, nullptr);
    }
}

void SharePlugin::finishedFileTransfer()
{
    auto job = qobject_cast<Job*>(QObject::sender());
    Q_ASSERT(job != nullptr);
    if (job->errorString().isEmpty()) {
        // TODO: add notification
        emit received(job->target().toString());
        qCDebug(logger)
                << "Share succeeded:" << job->target().toString();
    } else {
        qCWarning(logger)
                << "Share failed:"
                << job->errorString()
                << job->target().toString();
    }
}

QString SharePluginFactory::name() const
{
    return tr("Share");
}

QString SharePluginFactory::description() const
{
    return tr("Send and receive files and URLs.");
}

QString SharePluginFactory::iconUrl() const
{
    return "image://theme/icon-m-share";
}

void SharePluginFactory::registerTypes()
{
    qmlRegisterUncreatableType<SharePlugin>(
                "SailfishConnect.Share", 0, 3, "SharePlugin",
                QStringLiteral("not intented to be created from users"));
}

} // namespace SailfishConnect

Q_IMPORT_PLUGIN(SharePluginFactory)

