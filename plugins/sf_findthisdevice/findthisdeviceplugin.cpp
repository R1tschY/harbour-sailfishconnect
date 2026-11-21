/*
 * Copyright 2026 Manuel FLURY <zx97@users.noreply.github.com>.
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

#include "findthisdeviceplugin.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QLoggingCategory>
#include <QTimer>

#include <KPluginFactory>
#include <notification.h>

K_PLUGIN_CLASS_WITH_JSON(FindThisDevicePlugin, "sailfishconnect_findthisdevice.json")

Q_LOGGING_CATEGORY(loggerFindThisDevice, "kdeconnect.plugin.findthisdevice")

namespace {

// ngfd is the non-graphic feedback daemon of Sailfish OS. The service name
// carries a ".Backend" suffix but the interface does not.
const char ngfdService[] = "com.nokia.NonGraphicFeedback1.Backend";
const char ngfdPath[] = "/com/nokia/NonGraphicFeedback1";
const char ngfdInterface[] = "com.nokia.NonGraphicFeedback1";

// The "ringtone" event loops the ringtone selected by the user.
const char ringtoneEvent[] = "ringtone";

// The desktop client has no way to cancel the ringtone: it only sends the
// request. Stop by ourselves after one minute, the peer can ring again.
constexpr int maxRingTimeMs = 60 * 1000;

const QString packetTypeFindMyPhone =
    QStringLiteral("kdeconnect.findmyphone.request");

} // namespace

FindThisDevicePlugin::FindThisDevicePlugin(QObject *parent,
                                           const QVariantList &args)
    : KdeConnectPlugin(parent, args)
    , m_ngfd(new QDBusInterface(QString::fromLatin1(ngfdService),
                                QString::fromLatin1(ngfdPath),
                                QString::fromLatin1(ngfdInterface),
                                QDBusConnection::systemBus(),
                                this))
    , m_timeout(new QTimer(this))
{
    m_timeout->setSingleShot(true);
    m_timeout->setInterval(maxRingTimeMs);
    connect(m_timeout, &QTimer::timeout, this, &FindThisDevicePlugin::stopRinging);
}

FindThisDevicePlugin::~FindThisDevicePlugin()
{
    stopRinging();
}

void FindThisDevicePlugin::receivePacket(const NetworkPacket &np)
{
    if (np.type() != packetTypeFindMyPhone) {
        return;
    }

    if (np.get<bool>(QStringLiteral("isCancel"), false)) {
        stopRinging();
    } else {
        startRinging();
    }
}

void FindThisDevicePlugin::startRinging()
{
    // The desktop client has no separate cancel action: a second request
    // toggles the ringtone off.
    if (m_eventId != 0) {
        stopRinging();
        return;
    }

    if (!m_ngfd->isValid()) {
        qCWarning(loggerFindThisDevice) << "ngfd is not available:" << m_ngfd->lastError().message();
        return;
    }

    // Restart cleanly if a ringtone is already playing.
    stopRinging();

    QDBusReply<uint> reply = m_ngfd->call(QString::fromLatin1("Play"),
                                          QString::fromLatin1(ringtoneEvent),
                                          QVariantMap());
    if (!reply.isValid()) {
        qCWarning(loggerFindThisDevice) << "Cannot play ringtone:" << reply.error().message();
        return;
    }

    m_eventId = reply.value();
    m_timeout->start();

    // Let the user stop the ringtone from the device itself: tapping the
    // notification closes it, which stops the ringtone.
    m_notification = new Notification(this);
    m_notification->setAppName(tr("Sailfish Connect"));
    m_notification->setCategory(QStringLiteral("x-nemo.service.sailfishconnect"));
    m_notification->setUrgency(Notification::Critical);
    m_notification->setExpireTimeout(0);
    m_notification->setSummary(tr("Find my phone"));
    m_notification->setPreviewBody(tr("Tap to stop the ringtone"));
    m_notification->setIcon(QStringLiteral("icon-m-ringing"));
    // Tapping the notification stops the ringtone, even when the app page is
    // not in the foreground.
    m_notification->setRemoteActions({
        Notification::remoteAction(
            QStringLiteral("default"), tr("Stop"),
            QStringLiteral("org.harbour.SailfishConnect"),
            QStringLiteral("/org/harbour/SailfishConnect/UI"),
            QStringLiteral("org.harbour.SailfishConnect.UI"),
            QStringLiteral("stopFindPhone"), {device()->id()})});
    connect(m_notification, &Notification::closed, this,
            [this](uint reason) {
                Q_UNUSED(reason)
                stopRinging();
            });
    m_notification->publish();

    // Bring the ring page of the app to the foreground.
    notifyUi(QStringLiteral("showFindPhone"));
}

void FindThisDevicePlugin::stopRinging()
{
    m_timeout->stop();

    const uint eventId = m_eventId;
    m_eventId = 0;

    if (m_notification) {
        Notification *notification = m_notification;
        m_notification = nullptr;
        notification->close();
        notification->deleteLater();
    }

    if (eventId == 0 || !m_ngfd->isValid()) {
        return;
    }

    QDBusReply<uint> reply = m_ngfd->call(QString::fromLatin1("Stop"), eventId);
    if (!reply.isValid()) {
        qCWarning(loggerFindThisDevice) << "Cannot stop ringtone:" << reply.error().message();
    }

    notifyUi(QStringLiteral("hideFindPhone"));
}

void FindThisDevicePlugin::notifyUi(const QString &method)
{
    QDBusMessage message = QDBusMessage::createMethodCall(
        QStringLiteral("org.harbour.SailfishConnect"),
        QStringLiteral("/org/harbour/SailfishConnect/UI"),
        QStringLiteral("org.harbour.SailfishConnect.UI"), method);
    message << device()->id();
    QDBusConnection::sessionBus().asyncCall(message);
}

#include "findthisdeviceplugin.moc"
