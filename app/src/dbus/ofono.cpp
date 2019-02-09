#include "ofono.h"

#include "tuple.h"

namespace SailfishConnect {
namespace Ofono {

VoiceCall::VoiceCall(const QString path, QObject *parent)
    : QDBusAbstractInterface(
          QStringLiteral("org.ofono"),
          path,
          staticInterfaceName(),
          QDBusConnection::systemBus(),
          parent)
{ }

QDBusPendingReply<QVariantMap> VoiceCall::GetProperties()
{
    QList<QVariant> argumentList;
    return asyncCallWithArgumentList(
                QStringLiteral("GetProperties"), argumentList);
}

// -----------------------------------------------------------------------------

VoiceCallManager::VoiceCallManager(const QString path, QObject *parent)
    : QDBusAbstractInterface(
          QStringLiteral("org.ofono"),
          path,
          staticInterfaceName(),
          QDBusConnection::systemBus(),
          parent)
{ }

QDBusPendingReply<ObjectPropertiesArray> VoiceCallManager::GetCalls()
{
    QList<QVariant> argumentList;
    return asyncCallWithArgumentList(QStringLiteral("GetCalls"), argumentList);
}

// -----------------------------------------------------------------------------

Manager::Manager(QObject *parent)
    : QDBusAbstractInterface(
          QStringLiteral("org.ofono"),
          QStringLiteral("/"),
          staticInterfaceName(),
          QDBusConnection::systemBus(),
          parent)
{ }

QDBusPendingReply<ObjectPropertiesArray> Manager::GetModems()
{
    QList<QVariant> argumentList;
    return asyncCallWithArgumentList(QStringLiteral("GetModems"), argumentList);
}

// -----------------------------------------------------------------------------

void registerTypes() {
    qDBusRegisterMetaType<ObjectProperties>();
    qDBusRegisterMetaType<ObjectPropertiesArray>();
}

} // namespace Ofono
} // namespace SailfishConnect

