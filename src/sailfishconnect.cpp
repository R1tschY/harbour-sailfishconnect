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

#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif

#include <memory>
#include <QLoggingCategory>
#include <QQmlEngine>
#include <QtQml>
#include <QQuickView>
#include <QGuiApplication>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <sailfishapp.h>

#include <execinfo.h>
#include <stdlib.h>
#include <unistd.h>

#include "appdaemon.h"
#include "core/device.h"
#include "ui/devicelistmodel.h"
#include "ui/filtervalueproxymodel.h"
#include "ui/devicepluginsmodel.h"
#include "ui.h"

namespace SailfishConnect {

static Q_LOGGING_CATEGORY(logger, "sailfishconnect.ui")

QString DBUS_SERVICE_NAME =
        QStringLiteral("org.harbour.SailfishConnect");

QString PACKAGE_NAME =
        QStringLiteral("sailfishconnect");

QString PRETTY_PACKAGE_NAME =
        QStringLiteral("Sailfish-Connect");

void logBacktrace()
{
    void* array[32];
    size_t size = backtrace (array, 32);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
}

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = qFormatLogMessage(type, context, msg).toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
    case QtInfoMsg:
    case QtWarningMsg:
    case QtCriticalMsg:
        if (msg.contains("BACKTRACE")) {
            logBacktrace();
        }
        fprintf(stderr, "%s\n", localMsg.constData());
        break;
    case QtFatalMsg:
        fprintf(stderr, "%s\n", localMsg.constData());
        logBacktrace();
        abort();
    }
}

void registerQmlTypes() {
    qmlRegisterType<DeviceListModel>(
                "SailfishConnect.UI", 0, 1, "DeviceListModel");
    qmlRegisterType<FilterValueProxyModel>(
                "SailfishConnect.UI", 0, 1, "FilterValueProxyModel");
    qmlRegisterType<DevicePluginsModel>(
                "SailfishConnect.UI", 0, 1, "DevicePluginsModel");

    qmlRegisterType<Device>(
                "SailfishConnect.Core", 0, 1, "Device");
}

std::unique_ptr<QGuiApplication> createApplication(int &argc, char **argv)
{
    std::unique_ptr<QGuiApplication> app(SailfishApp::application(argc, argv));
    app->setApplicationDisplayName(PRETTY_PACKAGE_NAME);
    app->setApplicationName(PACKAGE_NAME);

    // always daemonize
    app->setQuitOnLastWindowClosed(false);

    return app;
}

} // SailfishConnect

int main(int argc, char *argv[])
{  
    using namespace SailfishConnect;

    qInstallMessageHandler(myMessageOutput);

    auto app = createApplication(argc, argv);

    auto sessionBus = QDBusConnection::sessionBus();
    if (!sessionBus.registerService(DBUS_SERVICE_NAME)) {
        qCInfo(logger) << "Other daemon exists.";
        UI::raise();
        return 0;
    }

    registerQmlTypes();

    AppDaemon daemon;
    UI ui;

    ui.showMainWindow();

    return app->exec();
}
