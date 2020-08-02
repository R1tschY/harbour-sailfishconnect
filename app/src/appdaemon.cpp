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

#include "appdaemon.h"

#include <QCoreApplication>
#include <QFile>
#include <QLoggingCategory>
#include <QQmlEngine>
#include <QSettings>
#include <QHostInfo>
#include <QTimer>
#include <KLocalizedString>
#include <notification.h>

#include <backends/pairinghandler.h>
#include <device.h>
#include <kdeconnectconfig.h>
#include "sailfishconnect-config.h"
#include "ui.h"
#include "io/jobmanager.h"


namespace SailfishConnect {

static Q_LOGGING_CATEGORY(logger, "sailfishconnect.ui")

AppDaemon::AppDaemon(QObject *parent)
    : Daemon(parent)
    , m_jobmanager(new JobManager(this))
{
    connect(
        &m_backgroundActivity, &BackgroundActivity::running,
        this, &AppDaemon::onWakeUp);
    m_backgroundActivity.setWakeupFrequency(BackgroundActivity::ThirtySeconds);

    onDeviceVisibilityChanged();
    connect(this, &Daemon::deviceVisibilityChanged,
            this, &AppDaemon::onDeviceVisibilityChanged);
}

void AppDaemon::askPairingConfirmation(Device *device)
{    
    Notification *notification = new Notification(this);

    notification->setAppName(QCoreApplication::applicationName());
    notification->setSummary(device->name());
    notification->setBody(i18n("Pending pairing request ..."));
    notification->setPreviewSummary(device->name());
    notification->setPreviewBody(i18n("Pairing request"));
    notification->setExpireTimeout(PairingHandler::pairingTimeoutMsec() * 0.75);
    notification->setRemoteActions(
                { UI::openDevicePageDbusAction(device->id()) });

    connect(notification, &Notification::closed,
            [=](uint reason) { notification->deleteLater(); });

    notification->publish();
    // TODO: remove/update notification when device is paired/unpaired
}

void AppDaemon::reportError(const QString& title, const QString& description)
{
    qCCritical(logger) << "Error to report:" << title << description;

    Notification *notification = new Notification(this);

    notification->setAppName(QCoreApplication::applicationName());
    notification->setSummary(PRETTY_PACKAGE_NAME);
    notification->setBody(description);
    notification->setPreviewSummary(PRETTY_PACKAGE_NAME);
    notification->setPreviewBody(description);

    connect(notification, &Notification::closed,
            [=](uint reason) { notification->deleteLater(); });

    notification->publish();
}

void AppDaemon::quit() {
    QCoreApplication::quit();
}

void AppDaemon::sendSimpleNotification(const QString &eventId, const QString &title, const QString &text, const QString &iconName) {
    qCInfo(logger) << "Notification:" << eventId << title << text;
    Notification *notification = new Notification(this);

    notification->setAppName(QCoreApplication::applicationName());
    notification->setSummary(title);
    notification->setBody(text);
    notification->setPreviewSummary(title);
    notification->setPreviewBody(text);
    if (eventId == QStringLiteral("pingReceived")) {
        notification->setIcon("image://theme/icon-lock-information");
    }
    connect(notification, &Notification::closed,
            [=](uint reason) { notification->deleteLater(); });

    notification->publish();
}

KJobTrackerInterface* AppDaemon::jobTracker() {
    return m_jobmanager;
}

QQmlImageProviderBase* AppDaemon::imageProvider(const QString& providerId) const
{
    if (!m_qmlEngine)
        return nullptr;

    return m_qmlEngine->imageProvider(providerId);
}

void AppDaemon::setQmlEngine(QQmlEngine* qmlEngine)
{
    if (m_qmlEngine == qmlEngine)
        return;

    if (m_qmlEngine) {
        m_qmlEngine->disconnect(this);
    }

    m_qmlEngine = qmlEngine;

    if (m_qmlEngine) {
        connect(m_qmlEngine, &QObject::destroyed,
            this, [this]() { setQmlEngine(nullptr); });
    }
}

void AppDaemon::onDeviceVisibilityChanged()
{
    QStringList connectedDevices = devices(true, true);
    qCDebug(logger)
        << "Device changed, got"
        << connectedDevices.size() << "connected devices";
    m_backgroundActivity.setState(
        connectedDevices.size()
        ? BackgroundActivity::Waiting : BackgroundActivity::Stopped);
}

void AppDaemon::onWakeUp()
{
   qCDebug(logger) << "Received wakeup";

    // immediately to go sleep, hope that is sufficient to keep connections
    // alive
    m_backgroundActivity.wait();
}

AppDaemon *AppDaemon::instance()
{
    return static_cast<AppDaemon*>(Daemon::instance());
}

} // namespace SailfishConnect
