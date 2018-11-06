/**
 * Copyright 2015 Holger Kaelberer <holger.k@elberer.de>
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusArgument>
#include <QtDebug>
#include <QLoggingCategory>
#include <QStandardPaths>
#include <QImage>
#include <QIcon>
#include <QLoggingCategory>
#include <QIODevice>
#include <QBuffer>
#include <QPainter>
#include <QQuickImageProvider>

#include <sailfishconnect/device.h>
#include <sailfishconnect/kdeconnectplugin.h>
#include <sailfishconnect/helper/cpphelper.h>
#include <appdaemon.h>

#include "notificationslistener.h"
#include "sendnotificationsplugin.h"
#include "notifyingapplication.h"

namespace SailfishConnect {

static Q_LOGGING_CATEGORY(logger, "SailfishConnect.SendNotifications")

static QSharedPointer<QIODevice> imageToPng(const QImage& image) {
    if (image.isNull())
        return QSharedPointer<QBuffer>();

    QSharedPointer<QBuffer> buffer;
    buffer->open(QIODevice::ReadWrite);

    bool success = image.save(buffer.data(), "PNG");
    if (success) {
        buffer->seek(0);
        return buffer;
    } else {
        return QSharedPointer<QBuffer>();
    }
}

NotificationsListener::NotificationsListener(KdeConnectPlugin* aPlugin)
    : QDBusAbstractAdaptor(aPlugin),
      m_plugin(aPlugin)
{
    qRegisterMetaTypeStreamOperators<NotifyingApplication>(
        "NotifyingApplication");

    bool ret = QDBusConnection::sessionBus()
        .registerObject(QStringLiteral("/org/freedesktop/Notifications"),
                        this,
                        QDBusConnection::ExportScriptableContents);
    if (!ret)
        qCWarning(logger)
                << "Error registering notifications listener for device"
                << m_plugin->device()->name() << ":"
                << QDBusConnection::sessionBus().lastError();
    else
        qCDebug(logger)
                << "Registered notifications listener for device"
                << m_plugin->device()->name();

    QDBusInterface iface(
                QStringLiteral("org.freedesktop.DBus"),
                QStringLiteral("/org/freedesktop/DBus"),
                QStringLiteral("org.freedesktop.DBus"));
    iface.call(
        QStringLiteral("AddMatch"),
        "interface='org.freedesktop.Notifications',member='Notify',type='method_call',eavesdrop='true'");

    loadApplications();

    connect(
        m_plugin->config(), &SailfishConnectPluginConfig::configChanged,
        this, &NotificationsListener::loadApplications);
}

NotificationsListener::~NotificationsListener()
{
    qCDebug(logger) << "Destroying NotificationsListener";
    QDBusInterface iface(
                QStringLiteral("org.freedesktop.DBus"),
                QStringLiteral("/org/freedesktop/DBus"),
                QStringLiteral("org.freedesktop.DBus"));
    QDBusMessage res = iface.call(
                QStringLiteral("RemoveMatch"),
                "interface='org.freedesktop.Notifications',member='Notify',type='method_call',eavesdrop='true'");
    QDBusConnection::sessionBus().unregisterObject(
                QStringLiteral("/org/freedesktop/Notifications"));
}

void NotificationsListener::loadApplications()
{
    m_applications.clear();
    const QVariantList list = m_plugin->config()->getList(
                QStringLiteral("applications"));
    for (const auto& a : list) {
        NotifyingApplication app = a.value<NotifyingApplication>();
        if (!m_applications.contains(app.name))
            m_applications.insert(app.name, app);
    }
    //qCDebug(logger) << "Loaded" << applications.size() << " applications";
}

bool NotificationsListener::parseImageDataArgument(const QVariant& argument,
                                                   int& width, int& height,
                                                   int& rowStride, int& bitsPerSample,
                                                   int& channels, bool& hasAlpha,
                                                   QByteArray& imageData) const
{
    if (!argument.canConvert<QDBusArgument>())
        return false;
    const QDBusArgument dbusArg = argument.value<QDBusArgument>();
    dbusArg.beginStructure();
    dbusArg >> width >> height >> rowStride >> hasAlpha >> bitsPerSample
            >> channels  >> imageData;
    dbusArg.endStructure();
    return true;
}

QSharedPointer<QIODevice> NotificationsListener::iconForImageData(
        const QVariant& argument) const
{
    int width, height, rowStride, bitsPerSample, channels;
    bool hasAlpha;
    QByteArray imageData;

    if (!parseImageDataArgument(argument, width, height, rowStride, bitsPerSample,
                                channels, hasAlpha, imageData))
        return QSharedPointer<QIODevice>();

    if (bitsPerSample != 8) {
        qCWarning(logger)
            << "Unsupported image format:"
            << "width=" << width
            << "height=" << height
            << "rowStride=" << rowStride
            << "bitsPerSample=" << bitsPerSample
            << "channels=" << channels
            << "hasAlpha=" << hasAlpha;
        return QSharedPointer<QIODevice>();
    }

    QImage image(reinterpret_cast<uchar*>(imageData.data()), width, height, rowStride,
                 hasAlpha ? QImage::Format_ARGB32 : QImage::Format_RGB32);
    if (hasAlpha)
        image = image.rgbSwapped();  // RGBA --> ARGB

    QSharedPointer<QBuffer> buffer = QSharedPointer<QBuffer>(new QBuffer);
    if (!buffer || !buffer->open(QIODevice::WriteOnly) ||
            !image.save(buffer.data(), "PNG")) {
        qCWarning(logger) << "Could not initialize image buffer";
        return QSharedPointer<QIODevice>();
    }

    return buffer;
}

static QSharedPointer<QIODevice> pathToPng(const QString& path) {
    if (path.endsWith(QLatin1String(".png"))) {
        return QSharedPointer<QIODevice>(new QFile(path));
    }

    auto buffer = imageToPng(QImage(path));
    if (!buffer) {
        qCWarning(logger)
                << "Could not convert image to png:"
                << path;
    }
    return buffer;
}

static QSharedPointer<QIODevice> themeIconToPng(const QString& iconName) {
    QIcon icon = QIcon::fromTheme(iconName);
    QPixmap pixmap = icon.pixmap(128);
    auto buffer = imageToPng(pixmap.toImage());
    if (!buffer) {
        qCWarning(logger)
                << "Could not convert theme icon to png:"
                << iconName;
    }
    return buffer;
}

QSharedPointer<QIODevice> NotificationsListener::iconForIconName(
        const QString& iconName) const
{
    qCDebug(logger) << "convert icon name" << iconName << "to png";

    if (!iconName.contains(QChar('/'))) {
        return themeIconToPng(iconName);
    } else {
        auto url = QUrl::fromUserInput(
            iconName, QString(), QUrl::AssumeLocalFile);

        QString scheme = url.scheme();
        if (scheme == QLatin1String("file")) {
            return pathToPng(url.path());
        } else if (scheme == QLatin1String("image")) {
            if (url.host() == QLatin1String("theme")) {
                return themeIconToPng(url.path());
            } else {
                // TODO: give up here?
                auto* imageProvider = static_cast<QQuickImageProvider*>(
                        AppDaemon::instance()->imageProvider(url.host()));
                if (!imageProvider) {
                    qCWarning(logger)
                            << "No image provider" << url.host() << "found.";
                    return QSharedPointer<QIODevice>();
                }

                QSize size;
                QSize requestedSize(128, 128);
                switch (imageProvider->imageType()) {
                case QQmlImageProviderBase::Image:
                    return imageToPng(imageProvider->requestImage(
                        url.path(), &size, requestedSize));
                case QQmlImageProviderBase::Pixmap:
                    return imageToPng(imageProvider->requestPixmap(
                        url.path(), &size, requestedSize).toImage());
                default:
                    qCWarning(logger)
                            << "Not supported QQuickImageProvider image type:"
                            << iconName;
                    return QSharedPointer<QIODevice>();
                }
            }
        } else {
            qCWarning(logger)
                    << "Not supported file scheme for icon file" << scheme;
            return QSharedPointer<QIODevice>();
        }
    }
}

uint NotificationsListener::Notify(const QString& appName, uint replacesId,
                                   const QString& appIcon,
                                   const QString& summary, const QString& body,
                                   const QStringList& actions,
                                   const QVariantMap& hints, int timeout)
{
    static int gid = 0; // TODO: add to class
    Q_UNUSED(actions);

    //qCDebug(logger) << "Got notification appName=" << appName << "replacesId=" << replacesId << "appIcon=" << appIcon << "summary=" << summary << "body=" << body << "actions=" << actions << "hints=" << hints << "timeout=" << timeout;

    // skip our own notifications
    if (hints.value(QStringLiteral("x-sailfishconnect-hide"), false).toBool())
        return 0;

    auto* config = m_plugin->config();

    NotifyingApplication app;
    if (!m_applications.contains(appName)) {
        // new application -> add to config
        app.name = appName;
        app.icon = appIcon;
        app.active = true;
        app.blacklistExpression = QRegularExpression();
        m_applications.insert(app.name, app);
        // update config:
        QVariantList list;
        for (const auto& a : asConst(m_applications))
            list << QVariant::fromValue<NotifyingApplication>(a);
        config->setList(QStringLiteral("applications"), list);
        //qCDebug(logger) << "Added new application to config:" << app;
    } else {
        app = m_applications.value(appName);
    }

    if (!app.active)
        return 0;

    if (timeout > 0 &&
            config->get(QStringLiteral("generalPersistent"), false))
        return 0;

    int urgency = -1;
    if (hints.contains(QStringLiteral("urgency"))) {
        bool ok;
        urgency = hints[QStringLiteral("urgency")].toInt(&ok);
        if (!ok)
            urgency = -1;
    }
    if (urgency > -1 &&
            urgency < config->get<int>(QStringLiteral("generalUrgency"), 0))
        return 0;

    gid += 1;
    int id = replacesId > 0 ? replacesId : gid;

    QString ticker = summary;
    if (!body.isEmpty() &&
            config->get(QStringLiteral("generalIncludeBody"), true))
        ticker += QStringLiteral(": ") + body;

    if (app.blacklistExpression.isValid() &&
            !app.blacklistExpression.pattern().isEmpty() &&
            app.blacklistExpression.match(ticker).hasMatch())
        return 0;

    //qCDebug(logger) << "Sending notification from" << appName << ":" <<ticker << "; appIcon=" << appIcon;
    NetworkPackage np("kdeconnect.notification", {
        {"id", QString::number(id)},
        {"appName", appName},
        {"ticker", ticker},
        {"isClearable", timeout == 0}
    });

    // sync any icon data?
    if (config->get(QStringLiteral("generalSynchronizeIcons"), true)) {
        QSharedPointer<QIODevice> iconSource;
        // try different image sources according to priorities in notifications-
        // spec version 1.2:
        if (hints.contains(QStringLiteral("image-data")))
            iconSource = iconForImageData(hints[QStringLiteral("image-data")]);
        // 1.1 backward compatibility
        else if (hints.contains(QStringLiteral("image_data")))
            iconSource = iconForImageData(hints[QStringLiteral("image_data")]);
        else if (hints.contains(QStringLiteral("image-path")))
            iconSource = iconForIconName(hints[QStringLiteral("image-path")].toString());
        // 1.1 backward compatibility
        else if (hints.contains(QStringLiteral("image_path")))
            iconSource = iconForIconName(hints[QStringLiteral("image_path")].toString());
        else if (hints.contains(QStringLiteral("x-nemo-icon")))
            iconSource = iconForIconName(hints[QStringLiteral("x-nemo-icon")].toString());
        else if (!appIcon.isEmpty())
            iconSource = iconForIconName(appIcon);
        // < 1.1 backward compatibility
        else if (hints.contains(QStringLiteral("icon_data")))
            iconSource = iconForImageData(hints[QStringLiteral("icon_data")]);

        if (iconSource)
            np.setPayload(iconSource, iconSource->size());
    }

    m_plugin->sendPackage(np);

    return id;
}

} // namespace SailfishConnect
