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

#include <libintl.h>
#include <execinfo.h>
#include <stdlib.h>
#include <unistd.h>

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
#include <KLocalizedString>
#include <kcoreaddons_version.h>
#include <sailfishapp.h>

#include "appdaemon.h"
#include <device.h>
#include <kdeconnectplugin.h>

#include <devicessortproxymodel.h>
#include <devicesmodel.h>
#include <notificationsmodel.h>
#include <remotecommandsmodel.h>
#include <remotesinksmodel.h>

#include "models/devicelistmodel.h"
#include "models/sortfiltermodel.h"
#include "models/devicepluginsmodel.h"
#include "models/mprisplayersmodel.h"
#include "models/stringlistmodel.h"

#include "ui.h"
#include "js/qmlregister.h"
#include "dbus/kdeconnect.h"
#include "dbus/servicewatcher.h"
#include "helper/jobsnotificator.h"
#include "helper/keyboardlayoutprovider.h"

// load QCA openssl plugin
Q_IMPORT_PLUGIN(opensslPlugin)

namespace SailfishConnect {

namespace {

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

void initI18n() {
#if KCOREADDONS_VERSION < QT_VERSION_CHECK(5,38,0)
    QByteArray dataDirs = qgetenv("XDG_DATA_DIRS");
    if (dataDirs.isEmpty()) {
        dataDirs = "/usr/local/share/:/usr/share/";
    }
    dataDirs.append(QStringLiteral(":/usr/share/%1/").arg(PACKAGE_NAME).toUtf8());

    if (!qputenv("XDG_DATA_DIRS", dataDirs)) {
        qCWarning(logger) << "Failed to set XDG_DATA_DIRS. Translations will be missing.";  
    }
#else
    QString localeDir = QStringLiteral("/usr/share/%1/locale").arg(PACKAGE_NAME);
    KLocalizedString::addDomainLocaleDir("kdeconnect-core", localeDir);
    KLocalizedString::addDomainLocaleDir("kdeconnect-plugins", localeDir);
    KLocalizedString::addDomainLocaleDir("sailfishconnect-app", localeDir);
    KLocalizedString::addDomainLocaleDir("sailfishconnect-plugins", localeDir);
#endif

    KLocalizedString::setApplicationDomain("sailfishconnect-app");

    qCDebug(logger) << "availableApplicationTranslations" << KLocalizedString::availableApplicationTranslations();  
    qCDebug(logger) << "languages" << KLocalizedString::languages();
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
                "SailfishConnect.UI", 0, 6, "DeviceListModel");
    qmlRegisterType<SortFilterModel>(
                "SailfishConnect.UI", 0, 6, "SortFilterModel");
    qmlRegisterType<DevicePluginsModel>(
                "SailfishConnect.UI", 0, 6, "DevicePluginsModel");
    qmlRegisterType<MprisPlayersModel>(
                "SailfishConnect.UI", 0, 6, "MprisPlayersModel");
    qmlRegisterType<StringListModel>(
                "SailfishConnect.UI", 0, 6, "StringListModel");
    qmlRegisterType<KeyboardLayoutProvider>(
                "SailfishConnect.UI", 0, 6, "KeyboardLayoutProvider");

    QString uncreatableError =
        QStringLiteral("Instances are only creatable from C++.");
        
    qmlRegisterUncreatableType<DaemonApi>(
                "SailfishConnect.Api", 0, 6, "DaemonApi", uncreatableError);

    qRegisterMetaType<DeviceApi*>("DeviceApi*");
    qmlRegisterUncreatableType<DeviceApi>(
                "SailfishConnect.Api", 0, 6, "DeviceApi", uncreatableError);

    qRegisterMetaType<RemoteControlApi*>("RemoteControlApi*");
    qmlRegisterUncreatableType<RemoteControlApi>(
                "SailfishConnect.Api", 0, 6, "RemoteControlApi", uncreatableError);

    qRegisterMetaType<RemoteKeyboardApi*>("RemoteKeyboardApi*");
    qmlRegisterUncreatableType<RemoteKeyboardApi>(
                "SailfishConnect.Api", 0, 6, "RemoteKeyboardApi", uncreatableError);

    qRegisterMetaType<RemoteCommandsApi*>("RemoteCommandsApi*");
    qmlRegisterUncreatableType<RemoteCommandsApi>(
                "SailfishConnect.Api", 0, 6, "RemoteCommandsApi", uncreatableError);

    qRegisterMetaType<RemoteSystemVolumeApi*>("RemoteSystemVolumeApi*");
    qmlRegisterUncreatableType<RemoteSystemVolumeApi>(
                "SailfishConnect.Api", 0, 6, "RemoteSystemVolumeApi", uncreatableError);

    qRegisterMetaType<RemoteCommandsApi*>("ShareApi*");
    qmlRegisterUncreatableType<RemoteCommandsApi>(
                "SailfishConnect.Api", 0, 6, "ShareApi", uncreatableError);

    qRegisterMetaType<PluginConfigApi*>("PluginConfigApi*");
    qmlRegisterUncreatableType<PluginConfigApi>(
                "SailfishConnect.Api", 0, 6, "PluginConfigApi", uncreatableError);

    qmlRegisterType<DBusServiceWatcher>(
                "SailfishConnect.Api", 0, 6, "DBusServiceWatcher");

    QmlJs::registerTypes();
}

bool copyDirectory(const QString& src, const QString& dst)
{
    QDir srcDir(src);
    if (!srcDir.exists()) {
        qCCritical(logger) << "Directory" << src << "does not exist";
        return false;
    }

    for (const QString dir : srcDir.entryList(
             QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks)) {
        QString dst_path = dst % '/' % dir;
        if (!srcDir.mkpath(dst_path)) {
            qCCritical(logger) << "Failed to create directory" << dst_path;
            return false;
        }
        if (!copyDirectory(src % '/' % dir, dst_path)) {
            return false;
        }
    }

    for (const QString file : srcDir.entryList(QDir::Files)) {
        if (!QFile::copy(src  % '/' % file, dst  % '/' % file)) {
            qCCritical(logger)
                << "Failed to copy file" << (src  % '/' % file)
                << "to" << (dst  % '/' % file);
            return false;
        }
    }

    return true;
}

void migrateOldInstallation() {
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    QString oldConfigPath = configPath
            % '/' % QStringLiteral("harbour-sailfishconnect")
            % '/' % QStringLiteral("harbour-sailfishconnect");
    QString newConfigPath = configPath
            % '/' % QStringLiteral("harbour-sailfishconnect");

    QString oldCertificatePath = oldConfigPath % '/' % QStringLiteral("certificate.pem");
    QString newCertificatePath = newConfigPath % '/' % QStringLiteral("certificate.pem");

    if (QFileInfo::exists(oldCertificatePath) && !QFileInfo::exists(newCertificatePath)) {
        qCInfo(logger) << "Migrate config from" << oldConfigPath << "to" << newConfigPath;
        if (copyDirectory(oldConfigPath, newConfigPath)) {
            QDir(oldConfigPath).removeRecursively();
        }
    }
}

std::unique_ptr<QGuiApplication> createApplication(int &argc, char **argv)
{
    std::unique_ptr<QGuiApplication> app(SailfishApp::application(argc, argv));
    app->setApplicationVersion(PACKAGE_VERSION);
    app->setApplicationDisplayName(i18n("Sailfish Connect"));
    app->setQuitLockEnabled(false);
    return app;
}

struct Options {
    bool daemonMode;
};

Options parseCommandLine(const QCoreApplication &app) {
    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("KDE-Connect client for Sailfish OS"));
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

    puts("   ____     _ ______     __   _____                       __");
    puts("  / __/__ _(_) / _(_)__ / /  / ___/__  ___  ___  ___ ____/ /_");
    puts(" _\\ \\/ _ `/ / / _/ (_-</ _ \\/ /__/ _ \\/ _ \\/ _ \\/ -_) __/ __/");
    puts("/___/\\_,_/_/_/_//_/___/_//_/\\___/\\___/_//_/_//_/\\__/\\__/\\__/");
    puts(QStringLiteral(" --- Version %1").arg(PACKAGE_VERSION).toUtf8().constData());
    fflush(stdout);

    // Logging
    qInstallMessageHandler(myMessageOutput);
    QLoggingCategory::setFilterRules("kdeconnect.*=true");

    // I18n
    initI18n();

    // Migration
    migrateOldInstallation();

    // Application
    auto app = createApplication(argc, argv);

    // Command line
    auto options = parseCommandLine(*app);

    // DBus
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (!sessionBus.registerService(DBUS_SERVICE_NAME)) {
        qCInfo(logger) << "Other daemon exists.";
        if (!options.daemonMode)
            UI::raise();
        return 0;
    }
    
    // QML
    registerQmlTypes();

    // UI
    //KeyboardLayoutProvider keyboardLayoutProvider;
    UI ui(options.daemonMode);
    ui.showMainWindow();
    if (!options.daemonMode)
        ui.showMainWindow();

    JobsNotificator jobNotificator(
        qobject_cast<JobManager*>(Daemon::instance()->jobTracker()));

    return app->exec();
}
