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

#include <QLoggingCategory>
#include <QTimer>
#include <QSettings>
#include <QFile>

#include "core/systeminfo.h"
#include "utils/cpphelper.h"

namespace SailfishConnect {

static Q_LOGGING_CATEGORY(logger, "sailfishconnect.ui")

namespace {

class SailfishOsConfig : public SystemInfo
{
public:
    QString defaultName() const override;
    QString deviceType() const override;
};

QString SailfishOsConfig::defaultName() const
{
    const QString hwReleaseFile = QStringLiteral("/etc/hw-release");
    // QSettings will crash if the file does not exist or can be created, like in this case by us in /etc.
    // E.g. in the SFOS SDK Emulator there is no such file, so check before to protect against the crash.
    if (QFile::exists(hwReleaseFile)) {
        QSettings hwRelease(hwReleaseFile, QSettings::IniFormat);
        auto hwName = hwRelease.value(QStringLiteral("NAME")).toString();
        if (!hwName.isEmpty()) {
            return hwName;
        }
    }

    return SystemInfo::defaultName();
}

QString SailfishOsConfig::deviceType() const
{
    // TODO: How to detect tablet?
    return QStringLiteral("phone");
}

} // namespace


AppDaemon::AppDaemon(QObject *parent)
: Daemon(makeUniquePtr<SailfishOsConfig>(), parent)
{
    notification_.setAppName("Sailfish-Connect");
    notification_.setCategory("device");
}

void AppDaemon::askPairingConfirmation(Device *device)
{    
    notification_.setSummary(
        tr("Pairing request from %1").arg(device->name()));
    notification_.setOrigin(device->name());
    notification_.publish();
}

void AppDaemon::reportError(const QString &title, const QString &description)
{
    qCCritical(logger) << title << description;
}


} // namespace SailfishConnect
