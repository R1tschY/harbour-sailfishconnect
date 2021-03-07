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

#include "ui.h"

#include <QDBusInterface>
#include <QDBusReply>
#include <QEventLoopLocker>
#include <QGuiApplication>
#include <QLoggingCategory>
#include <QProcess>
#include <QQmlContext>
#include <QQuickView>
#include <QSettings>
#include <KLocalizedContext>
#include <notification.h>
#include <sailfishapp.h>

#include "appdaemon.h"
#include "dbus/kdeconnect.h"
#include "../../plugins/sf_mprisremote/albumartcache.h"
#include "sailfishconnect-config.h"
#include "helper/keyboardlayoutprovider.h"

namespace SailfishConnect {

static Q_LOGGING_CATEGORY(logger, "sailfishconnect.ui");

const QString UI::DBUS_INTERFACE_NAME = QStringLiteral("org.harbour.SailfishConnect.UI");

const QString UI::DBUS_PATH = QStringLiteral("/org/harbour/SailfishConnect/UI");

#ifndef BUILD_FOR_HARBOUR
const QString SERVICE_FILE_LOCATION =
        QStringLiteral("/usr/share/harbour-sailfishconnect/harbour-sailfishconnect.service");
#endif

UI::UI(bool daemonMode, QObject *parent)
    : QObject(parent)
    , m_daemon(new AppDaemon())
    , m_daemonApi(new DaemonApi(this))
    , m_daemonMode(daemonMode)
    , m_keyboardLayoutProvider(new KeyboardLayoutProvider(this))
{
    m_settings.beginGroup(QStringLiteral("UI"));

    auto sessionBus = QDBusConnection::sessionBus();
    if (!sessionBus.registerObject(
            DBUS_PATH,
            DBUS_INTERFACE_NAME,
            this,
            QDBusConnection::ExportScriptableSlots)) {
        qCCritical(logger)
            << "Registering" << DBUS_INTERFACE_NAME
            << "on" << DBUS_PATH
            << "failed";
    }
}

UI::~UI() = default;

void UI::showMainWindow()
{
    if (m_view) {
        m_view->showFullScreen();
        return;
    }

    m_view = SailfishApp::createView();
    m_view->rootContext()->setContextObject(new KLocalizedContext(m_view));
    m_daemon->setQmlEngine(m_view->engine());
    AlbumArtProvider::registerImageProvider(m_view->engine());

    m_view->installEventFilter(this);

    setRunInBackground(
        m_settings.value("runInBackground", m_runInBackground).toBool());

    // view
    m_view->rootContext()->setContextProperty("daemon", m_daemonApi);
    m_view->rootContext()->setContextProperty("ui", this);
    m_view->rootContext()->setContextProperty("keyboardLayout", m_keyboardLayoutProvider);
    m_view->setSource(SailfishApp::pathToMainQml());
    m_view->showFullScreen();
}

void UI::quit()
{
    m_daemon.reset();
    QCoreApplication::quit();
}

void UI::openDevicePage(const QString& deviceId)
{
    if (m_daemon->getDevice(deviceId) == nullptr) {
        // TODO: create dummy device or show a notification
        qCCritical(logger)
            << "while opening device page: device with id"
            << deviceId << "does not exist.";
        return;
    }

    qCDebug(logger) << "opening device page" << deviceId;
    emit openingDevicePage(deviceId);
}

static void handleSystemCtlResult(QProcess* systemCtl, bool enabled) {
    systemCtl->deleteLater();

    // TODO: show a notification on error
    if (systemCtl->exitStatus() != QProcess::NormalExit) {
        qCCritical(logger)
            << "Unable to "
            << (enabled ? "register" : "unregister")
            << "service: systemctl crashed:"
            << systemCtl->errorString();
    }
    else if (systemCtl->exitCode() != 0) {
        qCCritical(logger).nospace()
            << "Unable to "
            << (enabled ? "register" : "unregister")
            << " service: systemctl exited with "
            << systemCtl->exitCode() << ": "
            << systemCtl->readAll();
    } else {
        qCDebug(logger) 
            << (enabled ? "Registered" : "Unregistered")
            << "service:"
            << systemCtl->readAll();
    }
}

void UI::onRegisteredService()
{
    handleSystemCtlResult(qobject_cast<QProcess*>(sender()), true);
}

void UI::onUnregisteredService()
{
    handleSystemCtlResult(qobject_cast<QProcess*>(sender()), false);
}

void UI::raise()
{
    auto sessionBus = QDBusConnection::sessionBus();

    QDBusInterface daemonInterface(
        DBUS_SERVICE_NAME,
        DBUS_PATH,
        DBUS_INTERFACE_NAME,
        sessionBus);

    if (!daemonInterface.isValid()) {
        qCCritical(logger) << "Cannot communicate with daemon";
    }

    QDBusReply<void> reply = daemonInterface.call(QStringLiteral("showMainWindow"));
    if (!reply.isValid()) {
        // TODO: send short notification
        qCCritical(logger)
            << "Daemon raise call failed:"
            << reply.error().name() << "/"
            << reply.error().message();
    }
}

bool UI::runInBackground()
{
    return m_runInBackground;
}

void UI::setRunInBackground(bool value)
{
    if (value == m_runInBackground)
        return;

    m_runInBackground = value;
    m_settings.setValue("runInBackground", value);
    m_settings.sync();

    qGuiApp->setQuitOnLastWindowClosed(!value);

#ifndef BUILD_FOR_HARBOUR
    QProcess* systemctl = new QProcess(this);
    systemctl->setProcessChannelMode(QProcess::MergedChannels);
    if (value) {
        connect(systemctl, SIGNAL(finished(int)), this, SLOT(onRegisteredService()));
        systemctl->start("systemctl", { "--user", "enable", SERVICE_FILE_LOCATION });
    } else {
        connect(systemctl, SIGNAL(finished(int)), this, SLOT(onUnregisteredService()));
        systemctl->start("systemctl", { "--user", "disable", SERVICE_FILE_LOCATION });
    }
#endif

    emit runInBackgroundChanged();
}

QVariant UI::openDevicePageDbusAction(const QString &deviceId)
{
    return Notification::remoteAction(
            QStringLiteral("default"), QString("default"),
            DBUS_SERVICE_NAME, UI::DBUS_PATH, UI::DBUS_INTERFACE_NAME,
            QStringLiteral("openDevicePage"), { deviceId });
}

bool UI::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::Close && !m_runInBackground) {
        quit();
    }

    return QObject::eventFilter(watched, event);
}

} // namespace SailfishConnect
