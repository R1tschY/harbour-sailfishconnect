#include "ui.h"

#include <QLoggingCategory>
#include <QGuiApplication>
#include <QQuickView>
#include <QDBusInterface>
#include <QDBusReply>
#include <QQmlContext>
#include <QSettings>

#include <sailfishapp.h>

#include "sailfishconnect.h"
#include "appdaemon.h"

namespace SailfishConnect {

static Q_LOGGING_CATEGORY(logger, "sailfishconnect.ui")

const QString UI::DBUS_INTERFACE_NAME =
        QStringLiteral("org.harbour.SailfishConnect.UI");

const QString UI::DBUS_PATH =
        QStringLiteral("/org/harbour/SailfishConnect/UI");


UI::UI(AppDaemon* daemon, QObject *parent)
    : QObject(parent)
    , m_daemon(daemon)
{
    m_settings.beginGroup(QStringLiteral("UI"));

    auto sessionBus = QDBusConnection::sessionBus();
    sessionBus.registerObject(
        DBUS_PATH,
        DBUS_INTERFACE_NAME,
        this,
        QDBusConnection::ExportScriptableSlots);
}


void UI::showMainWindow()
{
    if (m_view) {
        m_view->showFullScreen();
        return;
    }

    m_view = SailfishApp::createView();

    setRunInBackground(
        m_settings.value("runInBackground", m_runInBackground).toBool());

    connect(
        m_view, &QQuickView::destroyed,
        this, &UI::onMainWindowDestroyed);

    // view
    m_view->rootContext()->setContextProperty("daemon", m_daemon);
    m_view->rootContext()->setContextProperty("ui", this);
    m_view->setSource(SailfishApp::pathToMainQml());
    m_view->showFullScreen();
}

void UI::quit()
{
    QCoreApplication::quit();
}

void UI::openDevicePage(const QString &deviceId)
{
    if (m_daemon->getDevice(deviceId) == nullptr) {
        // TODO: create device
        qCWarning(logger)
                << "while opening device page: device with id"
                << deviceId << "does not exist.";
        return;
    }

    qCDebug(logger) << "opening device page" << deviceId;
    emit openingDevicePage(deviceId);
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

    QDBusReply<void> reply = daemonInterface.call(
                QLatin1String("showMainWindow"));
    if (!reply.isValid()) {
        qCWarning(logger)
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

    qGuiApp->setQuitOnLastWindowClosed(!value);

    if (value) {
        m_view->installEventFilter(this);
    } else {
        m_view->removeEventFilter(this);
    }

    emit runInBackgroundChanged();
}

QVariant UI::openDevicePageDbusAction(const QString &deviceId)
{
    return Notification::remoteAction(
            QStringLiteral("default"), QString("default"),
            DBUS_SERVICE_NAME, UI::DBUS_PATH, UI::DBUS_INTERFACE_NAME,
            QStringLiteral("openDevicePage"), { deviceId });
}

void UI::onMainWindowDestroyed()
{
    Q_ASSERT(m_view);

    m_view = nullptr;
}

bool UI::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Close) {
        Q_ASSERT(m_view);
        m_view->deleteLater();
    }

    return QObject::eventFilter(obj, event);
}

} // namespace SailfishConnect
