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

namespace SailfishConnect {

static Q_LOGGING_CATEGORY(logger, "sailfishconnect.ui")

void AppDaemon::askPairingConfirmation(Device *device)
{
    qCWarning(logger) << "askPairingConfirmation";
    //        KNotification* notification = new KNotification(QStringLiteral("pairingRequest"));
    //        notification->setIconName(QStringLiteral("dialog-information"));
    //        notification->setComponentName(QStringLiteral("kdeconnect"));
    //        notification->setText(i18n("Pairing request from %1", device->name().toHtmlEscaped()));
    //        notification->setActions(QStringList() << i18n("Accept") << i18n("Reject"));
    ////         notification->setTimeout(PairingHandler::pairingTimeoutMsec());
    //        connect(notification, &KNotification::action1Activated, device, &Device::acceptPairing);
    //        connect(notification, &KNotification::action2Activated, device, &Device::rejectPairing);
    //        notification->sendEvent();

    QTimer::singleShot(1000, this, [=](){ device->acceptPairing(); });
}

void AppDaemon::reportError(const QString &title, const QString &description)
{
    qCCritical(logger) << title << description;
}


} // namespace SailfishConnect
