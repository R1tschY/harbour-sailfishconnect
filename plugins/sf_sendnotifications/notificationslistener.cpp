/*
 * Copyright 2015 Holger Kaelberer <holger.k@elberer.de>
 * Copyright 2018 Richard Liebscher <richard.liebscher@gmail.com>.
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

#include <QLoggingCategory>
#include <QImage>
#include <QIcon>
#include <QIODevice>
#include <QBuffer>
#include <QQuickImageProvider>
#include <QFile>
#include <QDBusArgument>

#include <core/device.h>
#include <core/qtcompat_p.h>
#include <core/kdeconnectplugin.h>
#include <core/kdeconnectpluginconfig.h>
#include <core/daemon.h>

#include "notificationslistener.h"
#include "sendnotificationsplugin.h"
#include "notifyingapplication.h"

static Q_LOGGING_CATEGORY(logger, "SailfishConnect.SendNotifications")

const char* NOTIFY_SIGNATURE = "susssasa{sv}i";


QString becomeMonitor(DBusConnection* conn, const char* match) {
    // message
    DBusMessage* msg = dbus_message_new_method_call(
        DBUS_SERVICE_DBUS,
        DBUS_PATH_DBUS,
        DBUS_INTERFACE_MONITORING,
        "BecomeMonitor");
    Q_ASSERT(msg != nullptr);
    bool success;

    // arguments
    const char* matches[] = {match};
    const char** matches_ = matches;
    dbus_uint32_t flags = 0;

    success = dbus_message_append_args(
                msg,
                DBUS_TYPE_ARRAY, DBUS_TYPE_STRING, &matches_, 1,
                DBUS_TYPE_UINT32, &flags,
                DBUS_TYPE_INVALID);
    if (!success) {
        return QStringLiteral("Failed to call dbus_message_append_args");
    }

    // send
    // TODO: wait and check for error: dbus_connection_send_with_reply_and_block
    success = dbus_connection_send(conn, msg, nullptr);
    if (!success) {
        return QStringLiteral("Failed to call dbus_connection_send");
    }

    dbus_message_unref(msg);

    return QString();
}

extern "C" DBusHandlerResult handleMessageFromC(
        DBusConnection *connection, DBusMessage *message, void *user_data
) {
    __try {
        auto* self = static_cast<NotificationsListenerThread*>(user_data);
        self->handleMessage(message);
    } __catch (...) {
        Q_ASSERT_X(false,
                   "NotificationsListenerThread::handleMessage",
                   "Catched exception");
    }

    // Monitors must not allow libdbus to reply to messages,
    // so we eat the message.
    return DBUS_HANDLER_RESULT_HANDLED;
}

NotificationsListenerThread::NotificationsListenerThread() = default;

NotificationsListenerThread::~NotificationsListenerThread()
{
    quit();
    dbus_connection_unref(m_connection);
}

void NotificationsListenerThread::quit()
{
    if (m_connection) {
        dbus_connection_close(m_connection);
    }
}

void NotificationsListenerThread::run()
{
    DBusConnection* connection =
            dbus_bus_get_private(DBUS_BUS_SESSION, &lastError);
    if (lastError) {
       return;
    }
    Q_ASSERT(connection != nullptr);
    m_connection = connection;

    dbus_connection_set_route_peer_messages(connection, true);
    dbus_connection_set_exit_on_disconnect(connection, false);
    dbus_connection_add_filter(connection, handleMessageFromC, this, nullptr);

    QString error = becomeMonitor(
                connection,
                "interface='org.freedesktop.Notifications',"
                "member='Notify'");

    if (!error.isEmpty()) {
        qCWarning(logger).noquote()
                << "Failed to become a DBus monitor."
                << " No notifictions will be send."
                << " Error:" << error;
    }

    // wake up every minute to see if we are still connected
    while (dbus_connection_read_write_dispatch(connection, 60 * 1000))
      ;
}

void NotificationsListenerThread::handleMessage(
        DBusMessage *message)
{
    switch (dbus_message_get_type (message))
    {
      case DBUS_MESSAGE_TYPE_METHOD_CALL:
//        qCDebug(logger) << "DBUS_MESSAGE_TYPE_METHOD_CALL";
        break;
      case DBUS_MESSAGE_TYPE_METHOD_RETURN:
        qCDebug(logger) << "DBUS_MESSAGE_TYPE_METHOD_RETURN";
        break;
      case DBUS_MESSAGE_TYPE_ERROR:
        qCDebug(logger) << "DBUS_MESSAGE_TYPE_ERROR";
        break;
      case DBUS_MESSAGE_TYPE_SIGNAL:
        qCDebug(logger) << "DBUS_MESSAGE_TYPE_SIGNAL";
        break;
    }

    if (dbus_message_is_method_call(
                message, "org.freedesktop.Notifications", "Notify")) {
        handleNotifyCall(message);
    }
}

static unsigned nextUnsigned(DBusMessageIter* iter) {
    Q_ASSERT(dbus_message_iter_get_arg_type(iter) == DBUS_TYPE_UINT32);
    DBusBasicValue value;
    dbus_message_iter_get_basic(iter, &value);
    dbus_message_iter_next(iter);
    return value.u32;
}

static int nextInt(DBusMessageIter* iter) {
    Q_ASSERT(dbus_message_iter_get_arg_type(iter) == DBUS_TYPE_INT32);
    DBusBasicValue value;
    dbus_message_iter_get_basic(iter, &value);
    dbus_message_iter_next(iter);
    return value.i32;
}

static QString nextString(DBusMessageIter* iter) {
    Q_ASSERT(dbus_message_iter_get_arg_type(iter) == DBUS_TYPE_STRING);
    DBusBasicValue value;
    dbus_message_iter_get_basic(iter, &value);
    dbus_message_iter_next(iter);
    return QString::fromUtf8(value.str);
}

static QStringList nextStringList(DBusMessageIter* iter) {
    DBusMessageIter sub;
    dbus_message_iter_recurse(iter, &sub);
    dbus_message_iter_next(iter);

    QStringList list;
    while (dbus_message_iter_get_arg_type(&sub) != DBUS_TYPE_INVALID) {
        list.append(nextString(&sub));
    }
    return list;
}

static QVariant nextVariant(DBusMessageIter* iter) {
    int type = dbus_message_iter_get_arg_type(iter);
    if (type != DBUS_TYPE_VARIANT)
        return QVariant();

    DBusMessageIter sub;
    dbus_message_iter_recurse(iter, &sub);
    dbus_message_iter_next(iter);

    type = dbus_message_iter_get_arg_type(&sub);
    if (dbus_type_is_basic(type)) {
        DBusBasicValue value;
        dbus_message_iter_get_basic(&sub, &value);
        switch (type) {
        case DBUS_TYPE_BOOLEAN:
            return QVariant(value.bool_val);
        case DBUS_TYPE_INT16:
            return QVariant(value.i16);
        case DBUS_TYPE_INT32:
            return QVariant(value.i32);
        case DBUS_TYPE_INT64:
            return QVariant((qlonglong)value.i64);
        case DBUS_TYPE_UINT16:
            return QVariant(value.u16);
        case DBUS_TYPE_UINT32:
            return QVariant(value.u32);
        case DBUS_TYPE_UINT64:
            return QVariant((qulonglong)value.u64);
        case DBUS_TYPE_BYTE:
            return QVariant(value.byt);
        case DBUS_TYPE_DOUBLE:
            return QVariant(value.dbl);
        case DBUS_TYPE_STRING:
            return QVariant(QString::fromUtf8(value.str));
        }
    }

    qCWarning(logger)
            << "Unimplemented conversation of type"
            << QChar(type)
            << type;

    return QVariant();
}

static QVariantMap nextVariantMap(DBusMessageIter* iter) {
    DBusMessageIter sub;
    dbus_message_iter_recurse(iter, &sub);
    dbus_message_iter_next(iter);

    QVariantMap map;
    while (dbus_message_iter_get_arg_type(&sub) != DBUS_TYPE_INVALID) {
        DBusMessageIter entry;
        dbus_message_iter_recurse(&sub, &entry);
        dbus_message_iter_next(&sub);
        QString key = nextString(&entry);
        QVariant value = nextVariant(&entry);
        map.insert(key, value);
    }
    return map;
}

void NotificationsListenerThread::handleNotifyCall(DBusMessage *message)
{
    DBusMessageIter iter;
    dbus_message_iter_init(message, &iter);

    if (!dbus_message_has_signature(message, NOTIFY_SIGNATURE)) {
        qCWarning(logger).nospace()
                << "Call to Notify has wrong signature. Expected "
                << NOTIFY_SIGNATURE << ", got "
                << dbus_message_get_signature(message);
        return;
    }

    QString appName = nextString(&iter);
    uint replacesId = nextUnsigned(&iter);
    QString appIcon = nextString(&iter);
    QString summary = nextString(&iter);
    QString body = nextString(&iter);
    QStringList actions = nextStringList(&iter);
    QVariantMap hints = nextVariantMap(&iter);
    int timeout = nextInt(&iter);

    Q_EMIT Notify(
        appName, replacesId, appIcon, summary, body, actions, hints,
        timeout);
}

NotificationsListener::NotificationsListener(KdeConnectPlugin* aPlugin)
    : QObject(aPlugin)
    , m_plugin(aPlugin)
    , m_thread(new NotificationsListenerThread())
{
    qRegisterMetaTypeStreamOperators<NotifyingApplication>(
        "NotifyingApplication");

    loadApplications();

    connect(
        m_plugin->config(), &KdeConnectPluginConfig::configChanged,
        this, &NotificationsListener::loadApplications);
    connect(m_thread, &NotificationsListenerThread::Notify,
            this, &NotificationsListener::onNotify);

    m_thread->start();
}

NotificationsListener::~NotificationsListener() {
    m_thread->quit();
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
    qCDebug(logger) << "Loaded" << m_applications.size() << " applications";
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

static QByteArray imageToPng(const QImage& image) {
    if (image.isNull())
        return QByteArray();

    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);

    // TODO: cache icons
    bool success = image.save(&buffer, "PNG");
    if (success) {
        return buffer.data();
    } else {
        return QByteArray();
    }
}

static QSharedPointer<QIODevice> imageToPngBuffer(const QImage& image) {
    if (image.isNull())
        return QSharedPointer<QIODevice>();

    QSharedPointer<QBuffer> buffer;
    buffer->open(QIODevice::ReadWrite);

    bool success = image.save(buffer.data(), "PNG");
    if (success) {
        buffer->seek(0);
        return buffer;
    } else {
        return QSharedPointer<QIODevice>();
    }
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

    return imageToPngBuffer(image);
}

static QByteArray pathToPng(const QString& path) {
    if (path.endsWith(QLatin1String(".png"))) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            qCWarning(logger)
                    << "Could not open png:"
                    << path;
            return QByteArray();
        }
        return file.readAll();
    }

    auto buffer = imageToPng(QImage(path));
    if (buffer.isEmpty()) {
        qCWarning(logger)
                << "Could not convert image to png:"
                << path;
    }
    return buffer;
}

static QByteArray themeIconToPng(const QString& iconName) {
    QIcon icon = QIcon::fromTheme(iconName);
    QPixmap pixmap = icon.pixmap(128);
    auto buffer = imageToPng(pixmap.toImage());
    if (buffer.isEmpty()) {
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

    QByteArray result;
    if (!iconName.contains(QChar::fromLatin1('/'))) {
        result = themeIconToPng(iconName);
    } else {
        auto url = QUrl::fromUserInput(
            iconName, QString(), QUrl::AssumeLocalFile);

        QString scheme = url.scheme();
        if (scheme == QLatin1String("file")) {
            result = pathToPng(url.path());
#if false && SAILFISHOS
        } else if (scheme == QLatin1String("image")) {
            auto* imageProvider = static_cast<QQuickImageProvider*>(
                    AppDaemon::instance()->imageProvider(url.host()));
            if (!imageProvider) {
                qCWarning(logger)
                        << "No image provider" << url.host() << "found.";
                result = QByteArray();
            } else {
                QString id = url.path();
                id.remove(0, 1);

                QSize size;
                QSize requestedSize(128, 128);
                QImage image;
                switch (imageProvider->imageType()) {
                case QQmlImageProviderBase::Image:
                    qCDebug(logger)
                            << "Get image from QQmlImageProvider" << id;
                    image = imageProvider->requestImage(
                        id, &size, requestedSize);
                    break;
                case QQmlImageProviderBase::Pixmap:
                    qCDebug(logger)
                            << "Get pixmap from QQmlImageProvider" << id;
                    image = imageProvider->requestPixmap(
                        id, &size, requestedSize).toImage();
                    break;
                case QQmlImageProviderBase::Texture:
                    qCDebug(logger)
                            << "Get texture from QQmlImageProvider" << id;
                    image = imageProvider->requestTexture(
                        id, &size, requestedSize)->image();
                    break;
                default:
                    Q_UNREACHABLE();
                }

                if (image.isNull()) {
                    qCWarning(logger)
                            << "Could not get theme icon:"
                            << iconName;
                }

                result = imageToPng(image);
                if (result.isNull()) {
                    qCWarning(logger)
                            << "Could not convert theme icon to png:"
                            << iconName;
                }
            }
#endif
        } else {
            qCWarning(logger)
                    << "Not supported file scheme for icon file" << scheme;
            result = QByteArray();
        }
    }

    auto* buffer = new QBuffer();
    buffer->setData(result);
    return QSharedPointer<QIODevice>(buffer);
}

void NotificationsListener::onNotify(const QString& appName, uint replacesId,
                                     const QString& appIcon,
                                     const QString& summary, const QString& body,
                                     const QStringList& actions,
                                     const QVariantMap& hints, int timeout)
{
    Q_UNUSED(actions);

//    qCDebug(logger)
//            << "Got notification appName=" << appName
//            << "replacesId=" << replacesId
//            << "appIcon=" << appIcon
//            << "summary=" << summary
//            << "body=" << body
//            << "actions=" << actions
//            << "hints=" << hints
//            << "timeout=" << timeout
//            << "owner=" << hints.value(
//                   QStringLiteral("x-nemo-owner")).toString();

#ifdef SAILFISHOS
    // skip our own notifications
    if (hints.value(QStringLiteral("x-nemo-owner")).toString()
            == QStringLiteral("harbour-sailfishconnect"))
        return;
#endif

    // qCDebug(logger) << "Notification hints:" << hints << "Actions:" << actions;

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
        for (const auto& a : qAsConst(m_applications))
            list << QVariant::fromValue<NotifyingApplication>(a);
        config->setList(QStringLiteral("applications"), list);
        //qCDebug(logger) << "Added new application to config:" << app;
    } else {
        app = m_applications.value(appName);
    }

    if (!app.active)
        return;

    if (timeout > 0 &&
            config->getBool(QStringLiteral("generalPersistent"), false))
        return;

    int urgency = -1;
    if (hints.contains(QStringLiteral("urgency"))) {
        bool ok;
        urgency = hints[QStringLiteral("urgency")].toInt(&ok);
        if (!ok)
            urgency = -1;
    }
    if (urgency > -1 &&
            urgency < config->getInt(QStringLiteral("generalUrgency"), 0))
        return;

    if (summary.isEmpty())
        return;

    QString ticker = summary;
    if (!body.isEmpty() &&
            config->getBool(QStringLiteral("generalIncludeBody"), true))
        ticker += QStringLiteral(": ") + body;

    if (app.blacklistExpression.isValid() &&
            !app.blacklistExpression.pattern().isEmpty() &&
            app.blacklistExpression.match(ticker).hasMatch())
        return;


    bool silent = false;
    bool requestAnswer = false;

#ifdef SAILFISHOS
    // use x-nemo-timestamp
    if (hints[QStringLiteral("x-nemo-hidden")].toBool())
        return;
#endif

    // TODO: use nemo hints
    // TODO: if id == 0 use return value of call
    //qCDebug(logger) << "Sending notification from" << appName << ":" <<ticker << "; appIcon=" << appIcon;
    NetworkPacket np(QStringLiteral("kdeconnect.notification"), {
        {QStringLiteral("id"), QString::number(replacesId)},
        {QStringLiteral("appName"), appName},
        {QStringLiteral("ticker"), ticker},
        {QStringLiteral("isClearable"), timeout == -1},
        {QStringLiteral("title"), summary},
        {QStringLiteral("text"), body},
        {QStringLiteral("requestAnswer"), requestAnswer}, // TODO: request package
        {QStringLiteral("silent"), silent},
    });

    // sync any icon data?
    // Only send icon on first notify (replacesId == 0)
    if (config->getBool(QStringLiteral("generalSynchronizeIcons"), true)
            && replacesId == 0) {
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
        else if (!appIcon.isEmpty())
            iconSource = iconForIconName(appIcon);
        // < 1.1 backward compatibility
        else if (hints.contains(QStringLiteral("icon_data")))
            iconSource = iconForImageData(hints[QStringLiteral("icon_data")]);
#ifdef SAILFISHOS
        // Lipstick compatibility (deprecated)
        else if (hints.contains(QStringLiteral("x-nemo-icon")))
            iconSource = iconForIconName(hints[QStringLiteral("x-nemo-icon")].toString());
#endif

        if (iconSource) {
            np.setPayload(iconSource, iconSource->size());
            // TODO: np.set(QStringLiteral("payloadHash"), );
        }
    }

    m_plugin->sendPacket(np);
}
