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

#include "sailfishconnect-config.h"

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
#include <QDebug>
#include <sailfishapp.h>

#include <execinfo.h>
#include <stdlib.h>
#include <unistd.h>

#include "appdaemon.h"
#include <device.h>
#include <kdeconnectplugin.h>

#include <devicessortproxymodel.h>
#include <devicesmodel.h>
#include <notificationsmodel.h>
#include <remotecommandsmodel.h>
#include <remotesinksmodel.h>


//#include "plugins/mprisremote/mprisremoteplugin.h"
//#include "plugins/touchpad/touchpadplugin.h"
#include "models/devicelistmodel.h"
#include "models/sortfiltermodel.h"
#include "models/devicepluginsmodel.h"
// #include "models/mprisplayersmodel.h"
// #include "models/jobsmodel.h"

#include "ui.h"
#include "js/qmlregister.h"
#include "dbus/ofono.h"
#include "dbus/kdeconnect.h"
#include "dbus/servicewatcher.h"

#include <QtPlugin>

Q_IMPORT_PLUGIN(opensslPlugin)

namespace SailfishConnect {

static Q_LOGGING_CATEGORY(logger, "sailfishconnect.ui")

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = qFormatLogMessage(type, context, msg).toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
    case QtInfoMsg:
    case QtWarningMsg:
    case QtCriticalMsg:
        fprintf(stderr, "%s\n", localMsg.constData());
        break;
    case QtFatalMsg:
        fprintf(stderr, "%s\n", localMsg.constData());
        abort();
    }
}

void registerQmlTypes() {
    qmlRegisterType<DevicesModel>(
        "org.kde.kdeconnect", 1, 0, "DevicesModel");
    qmlRegisterType<NotificationsModel>(
        "org.kde.kdeconnect", 1, 0, "NotificationsModel");
    qmlRegisterType<RemoteCommandsModel>(
        "org.kde.kdeconnect", 1, 0, "RemoteCommandsModel");
    qmlRegisterType<DevicesSortProxyModel>(
        "org.kde.kdeconnect", 1, 0, "DevicesSortProxyModel");
    qmlRegisterType<RemoteSinksModel>(
        "org.kde.kdeconnect", 1, 0, "RemoteSinksModel");

    // TODO: register in plugin factories when possible
    qmlRegisterType<DeviceListModel>(
                "SailfishConnect.UI", 0, 3, "DeviceListModel");
    qmlRegisterType<SortFilterModel>(
                "SailfishConnect.UI", 0, 3, "SortFilterModel");
    qmlRegisterType<DevicePluginsModel>(
                "SailfishConnect.UI", 0, 3, "DevicePluginsModel");
    // qmlRegisterType<JobsModel>(
    //             "SailfishConnect.UI", 0, 3, "JobsModel");
    // qmlRegisterType<MprisPlayersModel>(
    //             "SailfishConnect.UI", 0, 3, "MprisPlayersModel");

    QString uncreatableError =
        QStringLiteral("Instances are only creatable from C++.");
        
    qmlRegisterUncreatableType<DaemonApi>(
                "SailfishConnect.Api", 0, 7, "DaemonApi", uncreatableError);

    qRegisterMetaType<DeviceApi*>("DeviceApi*");
    qmlRegisterUncreatableType<DeviceApi>(
                "SailfishConnect.Api", 0, 7, "DeviceApi", uncreatableError);

    qRegisterMetaType<RemoteControlApi*>("RemoteControlApi*");
    qmlRegisterUncreatableType<RemoteControlApi>(
                "SailfishConnect.Api", 0, 7, "RemoteControlApi", uncreatableError);

    qmlRegisterType<DBusServiceWatcher>(
                "SailfishConnect.Api", 0, 7, "DBusServiceWatcher");

    QmlJs::registerTypes();
}

std::unique_ptr<QGuiApplication> createApplication(int &argc, char **argv)
{
    std::unique_ptr<QGuiApplication> app(SailfishApp::application(argc, argv));
    app->setApplicationVersion(PACKAGE_VERSION);
    app->setQuitLockEnabled(false);
    return app;
}

namespace {

struct Options {
    bool daemonMode;
};

Options parseCommandLine(const QCoreApplication &app) {
    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("Alternative KDE-Connect client for Sailfish OS"));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption daemonOption(QStringList() << "d" << "daemon",
        QStringLiteral("Start application in daemon mode. "
                       "Window is not shown until a call without is flag."));
    parser.addOption(daemonOption);

    parser.process(app);

    return Options {
        parser.isSet(daemonOption)
    };
}

} // namespace

} // namespace SailfishConnect

int main(int argc, char *argv[])
{  
    using namespace SailfishConnect;

    qInstallMessageHandler(myMessageOutput);
    QLoggingCategory::setFilterRules("kdeconnect.core=true");

    auto app = createApplication(argc, argv);

    auto options = parseCommandLine(*app);

    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (!sessionBus.registerService(DBUS_SERVICE_NAME)) {
        qCInfo(logger) << "Other daemon exists.";
        if (!options.daemonMode)
            UI::raise();
        return 0;
    }

    Ofono::registerTypes();
    registerQmlTypes();

    //KeyboardLayoutProvider keyboardLayoutProvider;
    UI ui(options.daemonMode);
    ui.showMainWindow();
    if (!options.daemonMode)
        ui.showMainWindow();

    return app->exec();
}
