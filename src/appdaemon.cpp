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
