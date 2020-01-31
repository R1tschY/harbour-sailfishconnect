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

#include <contextproperty.h>
#include <core/networkpacket.h>
#include <core/qtcompat_p.h>

K_PLUGIN_CLASS_WITH_JSON(BatteryReportPlugin, "sailfishconnect_batteryreport.json")

BatteryReportPlugin::BatteryReportPlugin(QObject* parent, const QVariantList& args)
    : KdeConnectPlugin(parent, args),
      chargePercentage_(new ContextProperty(
        QStringLiteral("Battery.ChargePercentage"), this)),
      isCharging_(new ContextProperty(
        QStringLiteral("Battery.IsCharging"), this)),
      lowBattery_(new ContextProperty(
        QStringLiteral("Battery.LowBattery"), this))
{
    debounceTimer_.setInterval(100);
    debounceTimer_.setSingleShot(true);
    debounceTimer_.setTimerType(Qt::CoarseTimer);

    connect(chargePercentage_, &ContextProperty::valueChanged,
            &debounceTimer_, QOverload<>::of(&QTimer::start));
    connect(isCharging_, &ContextProperty::valueChanged,
            &debounceTimer_, QOverload<>::of(&QTimer::start));
    connect(lowBattery_, &ContextProperty::valueChanged,
            &debounceTimer_, QOverload<>::of(&QTimer::start));
    connect(&debounceTimer_, &QTimer::timeout,
            this, &BatteryReportPlugin::sendStatus);

    chargePercentage_->subscribe();
    isCharging_->subscribe();
    lowBattery_->subscribe();

    debounceTimer_.start();
}

bool BatteryReportPlugin::receivePacket(const NetworkPacket &np)
{
    if (np.get<bool>(QStringLiteral("request"))) {
        debounceTimer_.start();
    }

    return true;
}

void BatteryReportPlugin::connected()
{
    debounceTimer_.start();
}

void BatteryReportPlugin::sendStatus()
{
    setCheckedStatus(
        QStringLiteral("currentCharge"), chargePercentage_->value(), 0, 100);
    setCheckedStatus(
        QStringLiteral("isCharging"), isCharging_->value(), 0, 1);
    setCheckedStatus(
        QStringLiteral("thresholdEvent"), lowBattery_->value(),
        ThresholdNone, ThresholdBatteryLow);

    NetworkPacket packet(QStringLiteral("kdeconnect.battery"), m_state);
    sendPacket(packet);
}

void BatteryReportPlugin::setCheckedStatus(
        const QString &name,
        const QVariant &value,
        int min,
        int max)
{
    bool okay = false;
    auto intValue = value.toInt(&okay);
    if (okay && intValue >= min && intValue <= max) {
        m_state.insert(name, intValue);
    } else {
        m_state.remove(name);
    }
}

#include "batteryreportplugin.moc"
