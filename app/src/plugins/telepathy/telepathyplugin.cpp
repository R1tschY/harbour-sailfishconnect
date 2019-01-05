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


#include "telepathyplugin.h"

#include <QDBusInterface>
#include <QLoggingCategory>

namespace SailfishConnect {

static Q_LOGGING_CATEGORY(logger, "sailfishconnect.telepathy")

TelepathyPlugin::TelepathyPlugin(
        Device *device,
        const QString &name,
        const QSet<QString> &outgoingCapabilities)
    : KdeConnectPlugin(device, name, outgoingCapabilities)
{ }

bool TelepathyPlugin::receivePacket(const NetworkPacket &np)
{
    if (np.get<bool>("sendSms")) {
        QDBusInterface qmlmessages(
                    QStringLiteral("org.nemomobile.qmlmessages"),
                    QStringLiteral("/"),
                    QStringLiteral("org.nemomobile.qmlmessages"),
                    QDBusConnection::sessionBus());

        auto phoneNumber = np.get<QString>("phoneNumber");
        auto messageBody = np.get<QString>("messageBody");

        if (!qmlmessages.isValid()) {
            qCWarning(logger)
                    << "no connection to message app possible"
                    << qmlmessages.lastError().message();
            return true;
        }

        qmlmessages.callWithCallback(
                    QStringLiteral("startSMS"),
                    { QStringList(phoneNumber), messageBody },
                    this,
                    SLOT(startSmsSuccess()),
                    SLOT(startSmsError(const QDBusError&)));
    }

    return true;
}

void TelepathyPlugin::startSmsSuccess() { }

void TelepathyPlugin::startSmsError(const QDBusError &error)
{
    qCWarning(logger)
            << "could not start message:" << error.message();
}

QString TelepathyPluginFactory::name() const
{
    return tr("Send SMS");
}

QString TelepathyPluginFactory::description() const
{
    return tr("Allow sending of text messages.");
}

QString TelepathyPluginFactory::iconUrl() const
{
    return "image://theme/icon-m-sms";
}

} // namespace SailfishConnect

Q_IMPORT_PLUGIN(TelepathyPluginFactory)
