#include "ui.h"

#include <QLoggingCategory>
#include <QGuiApplication>
#include <QQuickView>
#include <QDBusInterface>
#include <QDBusReply>

#include <sailfishapp.h>

#include "sailfishconnect.h"

namespace SailfishConnect {

static Q_LOGGING_CATEGORY(logger, "sailfishconnect.ui")

const QString UI::DBUS_INTERFACE_NAME =
        QStringLiteral("org.harbour.SailfishConnect.UI");

const QString UI::DBUS_PATH =
        QStringLiteral("/org/harbour/SailfishConnect/UI");


UI::UI(QObject *parent) : QObject(parent)
{
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
    m_view->installEventFilter(this);

    connect(
        m_view, &QQuickView::destroyed,
        this, &UI::onMainWindowDestroyed);

    // view
    m_view->rootContext()->setContextProperty("daemon", this);
    m_view->setSource(SailfishApp::pathToMainQml());
    m_view->showFullScreen();
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
