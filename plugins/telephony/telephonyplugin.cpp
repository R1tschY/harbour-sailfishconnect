/*
 * Copyright 2019 Richard Liebscher <richard.liebscher@gmail.com>.
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


#include "telephonyplugin.h"

#include <QLoggingCategory>
#include <QDBusInterface>
#include <KPluginFactory>

#include <contactsmanager.h>

#include "ofono.h"
#include "tuple.h"

using namespace SailfishConnect;

K_PLUGIN_CLASS_WITH_JSON(TelephonyPlugin, "sailfishconnect_telephony.json")

static Q_LOGGING_CATEGORY(logger, "sailfishconnect.telephony")

static QString PACKET_TYPE_TELEPHONY = QStringLiteral("kdeconnect.telephony");

// -----------------------------------------------------------------------------

TelephonyCall::TelephonyCall(
        const QString& path, const QVariantMap &properties, QObject* parent)
    : QObject(parent)
    , m_obj(new Ofono::VoiceCall(path, this))
    , m_properties(properties)
{   
    connect(m_obj, &Ofono::VoiceCall::DisconnectReason,
            this, &TelephonyCall::disconnectReason);
    connect(m_obj, &Ofono::VoiceCall::PropertyChanged,
            this, &TelephonyCall::onPropertiesChanged);
}

QString TelephonyCall::state() const
{
    return m_properties.value(QStringLiteral("State")).toString();
}

QString TelephonyCall::lineIdentification() const
{
    return m_properties.value(QStringLiteral("LineIdentification")).toString();
}

QString TelephonyCall::path() const
{
    return m_obj->path();
}

void TelephonyCall::onPropertiesChanged(
        const QString &property, const QDBusVariant &value)
{
    m_properties[property] = value.variant();

    if (property == QStringLiteral("State")) {
        emit stateChanged();
    }
}

// -----------------------------------------------------------------------------

TelephonyPlugin::TelephonyPlugin(QObject* parent, const QVariantList& args)
    : KdeConnectPlugin(parent, args)
{
    Ofono::registerTypes();

    m_modem_manager = new Ofono::Manager(this);

    QDBusPendingReply<Ofono::ObjectPropertiesArray> reply =
            m_modem_manager->GetModems();
    reply.waitForFinished();
    if (reply.isError()) {
        qCCritical(logger) << "Fetching modems failed:" << reply.error();
        return;
    } else {
        const Ofono::ObjectPropertiesArray modems = reply.value();
        for (auto& modem : modems) {
            onModemAdded(std::get<0>(modem), std::get<1>(modem));
        }
    }

    connect(m_modem_manager, &Ofono::Manager::ModemAdded,
            this, &TelephonyPlugin::onModemAdded);
    connect(m_modem_manager, &Ofono::Manager::ModemRemoved,
            this, &TelephonyPlugin::onModemRemoved);
}

bool TelephonyPlugin::receivePacket(const NetworkPacket &np)
{
    // TODO: handle mute request
    Q_UNUSED(np);
    return false;
}

void TelephonyPlugin::onModemAdded(
        const QDBusObjectPath &path, const QVariantMap &properties)
{
    QString p = path.path();

    Ofono::VoiceCallManager* vcm = new Ofono::VoiceCallManager(p, this);
    connect(vcm, &Ofono::VoiceCallManager::CallAdded,
            this, &TelephonyPlugin::onCallAdded);
    connect(vcm, &Ofono::VoiceCallManager::CallRemoved,
            this, &TelephonyPlugin::onCallRemoved);
    QDBusPendingReply<Ofono::ObjectPropertiesArray> reply = vcm->GetCalls();
    reply.waitForFinished();
    if (reply.isError()) {
        qCWarning(logger) << "Fetching calls failed:" << reply.error();
    } else {
        const Ofono::ObjectPropertiesArray calls = reply.value();
        for (auto& call : calls) {
            onCallAdded(std::get<0>(call), std::get<1>(call));
        }
    }

    m_modems.insert(p);
    m_voice_call_managers.insert(p, vcm);
}

void TelephonyPlugin::onModemRemoved(const QDBusObjectPath &path)
{
    QString p = path.path();

    m_modems.remove(p);
    m_voice_call_managers.take(p)->deleteLater();
    // TODO: remove voice calls
}

void TelephonyPlugin::onCallAdded(
        const QDBusObjectPath &path, const QVariantMap &properties)
{
    QString p = path.path();
    qCDebug(logger) << "Call added" << p;

    TelephonyCall* tc = new TelephonyCall(p, properties, this);
    connect(tc, &TelephonyCall::stateChanged,
            this, &TelephonyPlugin::onCallStateChanged_);
    onCallStateChanged(tc);

    m_calls.insert(p, tc);
}

void TelephonyPlugin::onCallRemoved(const QDBusObjectPath &path)
{
    qCDebug(logger) << "Call removed" << path.path();
    TelephonyCall* call = m_calls.take(path.path());
    if (!call) return;

    call->deleteLater();
}

void TelephonyPlugin::onCallStateChanged_()
{
    onCallStateChanged(qobject_cast<TelephonyCall*>(QObject::sender()));
}

void TelephonyPlugin::onCallStateChanged(TelephonyCall* call)
{
    QString state = call->state();
    qCDebug(logger) << "Call state changed" << call->path() << state;

    if (state == QStringLiteral("incoming")) {
        // incomming call
        sendTelephonyPacket(QStringLiteral("ringing"), call);

    } else if (state == QStringLiteral("active")
               || state == QStringLiteral("dealing")
               || state == QStringLiteral("alerting"))
    {
        // active call or
        // outgoing call (rebuilded KDE Connect Android behavior)
        sendTelephonyPacket(QStringLiteral("talking"), call);

    } else if (state == QStringLiteral("disconnected")) {
        // call ended
        sendCancelTelephonyPacket(call);
    }
}

void TelephonyPlugin::sendTelephonyPacket(
        const QString& event, TelephonyCall* call)
{
    Q_ASSERT(call != nullptr);
    QString callPath = call->path();

    if (m_send_call_state.value(callPath) == event)
        return; // no change

    m_send_call_state[callPath] = event;

    QString phoneNumber = call->lineIdentification();
    QString displayName = ContactsManager::instance().lookUpName(phoneNumber);

    NetworkPacket np{
        PACKET_TYPE_TELEPHONY,
        {
            { QStringLiteral("event"), event },
            { QStringLiteral("phoneNumber"), phoneNumber },
            { QStringLiteral("contactName"), displayName },
        }
    };
    sendPacket(np);
}

void TelephonyPlugin::sendCancelTelephonyPacket(TelephonyCall* call)
{
    Q_ASSERT(call != nullptr);

    QString callPath = call->path();
    auto lastState = m_send_call_state.value(
                callPath, QStringLiteral("talking"));
    m_send_call_state.remove(callPath);

    QString phoneNumber = call->lineIdentification();
    QString displayName = ContactsManager::instance().lookUpName(phoneNumber);

    NetworkPacket np{
        PACKET_TYPE_TELEPHONY,
        {
            { QStringLiteral("event"), lastState },
            { QStringLiteral("phoneNumber"), phoneNumber },
            { QStringLiteral("contactName"), displayName },
            { QStringLiteral("isCancel"), true }
        }
    };
    sendPacket(np);
}

#include "telephonyplugin.moc"
