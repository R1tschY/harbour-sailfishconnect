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

#include "batteryreportplugin.h"

#include <KPluginFactory>
#include <QLoggingCategory>

#include <core/networkpacket.h>
#include <core/qtcompat_p.h>

#include <mce_signal.h>
#include <mce_request.h>

K_PLUGIN_CLASS_WITH_JSON(BatteryReportPlugin, "sailfishconnect_batteryreport.json")

static Q_LOGGING_CATEGORY(logger, "kdeconnect.plugin.batteryreport");

static QString MCE_SERVICE = QStringLiteral("com.nokia.mce");
static QString MCE_REQUEST_PATH = QStringLiteral("/com/nokia/mce/request");
static QString MCE_SIGNAL_PATH = QStringLiteral("/com/nokia/mce/signal");

class MceBatteryInfo: public QObject {
    Q_OBJECT
public: 
    MceBatteryInfo(QObject* parent = nullptr) 
        : QObject(parent)
        , m_bus(QDBusConnection::systemBus())
        , m_mceRequest(MCE_SERVICE, MCE_REQUEST_PATH, m_bus)    
        , m_mceSignal(MCE_SERVICE, MCE_SIGNAL_PATH, m_bus)
    {
        if (!m_mceRequest.isValid()) {
            qCWarning(logger) << "Failed to connect to MCE request interface:" 
                << m_mceRequest.lastError().message();
            return;
        }
        if (!m_mceSignal.isValid()) {
            qCWarning(logger) << "Failed to connect to MCE signal interface:" 
                << m_mceSignal.lastError().message();
            return;
        }

        QDBusReply<QString> ownerName = m_bus.interface()->call(QStringLiteral("GetNameOwner"), MCE_SERVICE);
        if (ownerName.isValid()) {
            m_nameOwner = ownerName;
        } else {
            qCWarning(logger).noquote() << "Failed to read MCE service owner:" << ownerName.error().message();
            return;
        }

        auto* serviceWatcher = new QDBusServiceWatcher(
            MCE_SERVICE, m_bus, QDBusServiceWatcher::WatchForOwnerChange, this);
        connect(serviceWatcher, &QDBusServiceWatcher::serviceOwnerChanged,
                this, &MceBatteryInfo::onServiceOwnerChanged);
        connect(&m_mceSignal, &ComNokiaMceSignalInterface::battery_level_ind,
                this, [this](int value) { 
                    m_currentCharge = value;
                    emit chargingStateChanged();
                });
        connect(&m_mceSignal, &ComNokiaMceSignalInterface::charger_state_ind,
                this, [this](const QString& value) { 
                    updateIsCharging(value);
                    emit chargingStateChanged();
                });
        connect(&m_mceSignal, &ComNokiaMceSignalInterface::battery_status_ind,
                this, [this](const QString& value) { 
                    updateLowCharge(value);
                    emit chargingStateChanged();
                });

        refresh();
    }

    void onServiceOwnerChanged(const QString& serviceName, const QString& old, const QString& new_) {
        if (serviceName != MCE_SERVICE)
            return;

        m_nameOwner = new_;
        if (!m_nameOwner.isEmpty()) {
            refresh();
        } else {
            m_isValid = false;
            emit chargingStateChanged();
        }
    }

    void refresh() {
        bool isValid = true;

        if (m_nameOwner.isEmpty()) {
            return;
        }

        QDBusReply<int> currentCharge = m_mceRequest.get_battery_level();
        if (currentCharge.isValid()) {
            m_currentCharge = currentCharge.value();
        } else {
            isValid = false;
            qCWarning(logger).noquote() << "Failed to read charge:" << currentCharge.error().message();
        }

        QDBusReply<QString> chargerState = m_mceRequest.get_charger_state();
        if (chargerState.isValid()) {
            updateIsCharging(chargerState.value());
        } else {
            isValid = false;
            qCWarning(logger).noquote() << "Failed to read charger state:" << chargerState.error().message();
        }

        QDBusReply<QString> batteryStatus = m_mceRequest.get_battery_status();
        if (batteryStatus.isValid()) {
            updateLowCharge(batteryStatus.value());
        } else {
            isValid = false;
            qCWarning(logger).noquote() << "Failed to read battery status:" << batteryStatus.error().message();
        }

        m_isValid = isValid;
        emit chargingStateChanged();
    }

    void updateIsCharging(const QString& value) {
        m_isCharging = value == QStringLiteral("on");
    }

    void updateLowCharge(const QString& value) {
        m_lowCharge = value == QStringLiteral("low");
    }

    int currentCharge() const {
        return m_currentCharge;
    }
    
    bool isCharging() const {
        return m_isCharging;
    }
    
    bool hasLowCharge() const {
        return m_lowCharge;
    }

    bool isValid() const {
        return m_isValid;
    }

signals:
    void chargingStateChanged();

private:
    QDBusConnection m_bus;
    ComNokiaMceRequestInterface m_mceRequest;
    ComNokiaMceSignalInterface m_mceSignal;

    int m_currentCharge = -1;
    bool m_isCharging = false;
    bool m_lowCharge = false;
    bool m_isValid = false;
    QString m_nameOwner;
};


BatteryReportPlugin::BatteryReportPlugin(QObject* parent, const QVariantList& args)
    : KdeConnectPlugin(parent, args)
    , m_batteryInfo(new MceBatteryInfo())
{
    connect(m_batteryInfo, &MceBatteryInfo::chargingStateChanged,
            this, &BatteryReportPlugin::sendStatus);
}

void BatteryReportPlugin::receivePacket(const NetworkPacket &np)
{
    if (np.get<bool>(QStringLiteral("request"))) {
        sendStatus();
    }
}

void BatteryReportPlugin::connected()
{
    sendStatus();
}

void BatteryReportPlugin::sendStatus()
{
    if (!m_batteryInfo->isValid())
        return;

    QVariantMap state;
    state.insert(QStringLiteral("currentCharge"), m_batteryInfo->currentCharge());
    state.insert(QStringLiteral("isCharging"), m_batteryInfo->isCharging());
    state.insert(
        QStringLiteral("thresholdEvent"), 
        m_batteryInfo->hasLowCharge() ? ThresholdBatteryLow : ThresholdNone);

    NetworkPacket packet(QStringLiteral("kdeconnect.battery"), state);
    sendPacket(packet);
}

#include "batteryreportplugin.moc"
