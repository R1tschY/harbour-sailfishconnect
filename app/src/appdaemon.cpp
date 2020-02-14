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
#include <QTimer>

#include <backends/pairinghandler.h>
//#include <systeminfo.h>
//#include <helper/cpphelper.h>
#include <device.h>
#include "sailfishconnect-config.h"
#include "ui.h"
#include "helper/contactsmanager.h"
#include "io/jobmanager.h"


namespace SailfishConnect {

static Q_LOGGING_CATEGORY(logger, "sailfishconnect.ui")

namespace {

// class SailfishOsConfig : public SystemInfo
// {
// public:
//     QString defaultName() const override;
//     QString deviceType() const override;
// };

// QString SailfishOsConfig::defaultName() const
// {
//     const QString hwReleaseFile = QStringLiteral("/etc/hw-release");
//     // QSettings will crash if the file does not exist or can be created, like in this case by us in /etc.
//     // E.g. in the SFOS SDK Emulator there is no such file, so check before to protect against the crash.
//     if (QFile::exists(hwReleaseFile)) {
//         QSettings hwRelease(hwReleaseFile, QSettings::IniFormat);
//         auto hwName = hwRelease.value(QStringLiteral("NAME")).toString();
//         if (!hwName.isEmpty()) {
//             return hwName;
//         }
//     }

//     return SystemInfo::defaultName();
// }

// QString SailfishOsConfig::deviceType() const
// {
//     // TODO: How to detect tablet?
//     return QStringLiteral("phone");
// }

} // namespace


AppDaemon::AppDaemon(QObject *parent)
: Daemon(parent)
, m_jobmanager(new JobManager(this))
, m_contacts(new ContactsManager(this))
{
    // notification_.setAppName(PRETTY_PACKAGE_NAME);
    // notification_.setCategory("device");
}

void AppDaemon::askPairingConfirmation(Device *device)
{    
    // Notification *notification = new Notification(this);

    // notification->setAppName(QCoreApplication::applicationName());
    // notification->setSummary(device->name());
    // notification->setBody(tr("Pending pairing request ..."));
    // notification->setPreviewSummary(device->name());
    // notification->setPreviewBody(tr("Pairing request"));
    // notification->setExpireTimeout(PairingHandler::pairingTimeoutMsec() * 0.75);
    // notification->setRemoteActions(
    //             { UI::openDevicePageDbusAction(device->id()) });

    // connect(notification, &Notification::closed,
    //         [=](uint reason) { notification->deleteLater(); });

    // notification->publish();
}

void AppDaemon::reportError(const QString& title, const QString& description)
{
    qCCritical(logger) << title << description;
}

void AppDaemon::quit() {

}

void AppDaemon::sendSimpleNotification(const QString&, const QString&, const QString&, const QString&) {

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

AppDaemon* AppDaemon::instance()
{
    return static_cast<AppDaemon*>(Daemon::instance());
}

} // namespace SailfishConnect
