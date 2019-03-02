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

#pragma once

#include <QObject>
#include <QtPlugin>
#include <QMap>
#include <QSet>
#include <QList>
#include <QVariant>

#include <sailfishconnect/kdeconnectplugin.h>

class QDBusPendingCallWatcher;
class QDBusInterface;
class QDBusError;
class QDBusObjectPath;
class QDBusVariant;

namespace SailfishConnect {

namespace Ofono {
class Manager;
class VoiceCallManager;
class VoiceCall;
} // namespace Ofono


class TelephonyCall : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString state READ state NOTIFY stateChanged)
public:
    TelephonyCall(
            const QString& path, const QVariantMap &properties, QObject* parent);

    QString state() const;
    QString lineIdentification() const;
    QString path() const;

signals:
    void disconnectReason(const QString& reason);
    void stateChanged();

private:
    Ofono::VoiceCall* m_obj;
    QVariantMap m_properties;

    void onPropertiesChanged(
            const QString& property, const QDBusVariant& value);
};


class TelephonyPlugin : public KdeConnectPlugin
{
    Q_OBJECT
public:
    TelephonyPlugin(
            Device* device,
            const QString &name,
            const QSet<QString> &outgoingCapabilities);

public slots:
    bool receivePacket(const NetworkPacket &np) override;

private:
    Ofono::Manager* m_modem_manager;
    QSet<QString> m_modems;
    QMap<QString, TelephonyCall*> m_calls;
    QMap<QString, Ofono::VoiceCallManager*> m_voice_call_managers;
    QMap<QString, QString> m_send_call_state;

    void onModemAdded(const QDBusObjectPath &path, const QVariantMap &properties);
    void onModemRemoved(const QDBusObjectPath &path);

    void onCallAdded(const QDBusObjectPath &path, const QVariantMap &properties);
    void onCallRemoved(const QDBusObjectPath &path);

    void onCallStateChanged_();
    void onCallStateChanged(TelephonyCall* call);

    void sendTelephonyPacket(const QString& event, TelephonyCall *call);
    void sendCancelTelephonyPacket(TelephonyCall* call);
};


class TelephonyPluginFactory :
        public SailfishConnectPluginFactory_<TelephonyPlugin>
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID SailfishConnectPlugin_iid FILE "metadata.json")
    Q_INTERFACES(SailfishConnectPluginFactory)
public:
    using FactoryBaseType::FactoryBaseType;

    QString name() const override;
    QString description() const override;
    QString iconUrl() const override;
};


} // namespace SailfishConnect
