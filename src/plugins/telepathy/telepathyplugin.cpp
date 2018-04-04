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
    , m_qmlmessages(new QDBusInterface(
                        QStringLiteral("org.nemomobile.qmlmessages"),
                        QStringLiteral("/"),
                        QStringLiteral("org.nemomobile.qmlmessages"),
                        QDBusConnection::sessionBus(),
                        this))
{ }

bool TelepathyPlugin::receivePackage(const NetworkPackage &np)
{
    if (np.get<bool>("sendSms")) {
        auto phoneNumber = np.get<QString>("phoneNumber");
        auto messageBody = np.get<QString>("messageBody");

        if (!m_qmlmessages->isValid()) {
            qCWarning(logger)
                    << "no connection to message app possible"
                    << m_qmlmessages->lastError().message();
            return true;
        }

        m_qmlmessages->callWithCallback(
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
    return tr("Send text messages from other devices.");
}

QString TelepathyPluginFactory::iconUrl() const
{
    return "image://theme/icon-m-sms";
}

} // namespace SailfishConnect

Q_IMPORT_PLUGIN(TelepathyPluginFactory)
