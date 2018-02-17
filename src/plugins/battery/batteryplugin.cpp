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

#include "batteryplugin.h"

#include <QtGlobal>

#include <contextproperty.h>
#include <core/networkpackage.h>
#include <utils/cpphelper.h>


namespace SailfishConnect {

BatteryPlugin::BatteryPlugin(
        Device *device, QString name, QSet<QString> outgoingCapabilities)
    : KdeConnectPlugin(device, name, outgoingCapabilities),
      batteryPackage_(QStringLiteral("kdeconnect.battery")),
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
            &debounceTimer_, Overload<>::of(&QTimer::start));
    connect(isCharging_, &ContextProperty::valueChanged,
            &debounceTimer_, Overload<>::of(&QTimer::start));
    connect(lowBattery_, &ContextProperty::valueChanged,
            &debounceTimer_, Overload<>::of(&QTimer::start));
    connect(&debounceTimer_, &QTimer::timeout,
            this, &BatteryPlugin::sendStatus);

    chargePercentage_->subscribe();
    isCharging_->subscribe();
    lowBattery_->subscribe();
}

bool BatteryPlugin::receivePackage(const NetworkPackage &np)
{
    if (np.get<bool>(QStringLiteral("request"))) {
        debounceTimer_.start();
    }

    return true;
}

void BatteryPlugin::connected()
{
    debounceTimer_.start();
}

void BatteryPlugin::sendStatus()
{
    setCheckedStatus(
        QStringLiteral("currentCharge"), chargePercentage_->value(), 0, 100);
    setCheckedStatus(
        QStringLiteral("isCharging"), isCharging_->value(), 0, 1);
    setCheckedStatus(
        QStringLiteral("thresholdEvent"), lowBattery_->value(),
        ThresholdNone, ThresholdBatteryLow);

    sendPackage(batteryPackage_);
}

void BatteryPlugin::setCheckedStatus(
        const QString &name,
        const QVariant &value,
        int min,
        int max)
{
    bool okay = false;
    auto intValue = value.toInt(&okay);
    if (okay && intValue >= min && intValue <= max) {
        batteryPackage_.set(name, intValue);
    } else {
        batteryPackage_.remove(name);
    }
}

QString BatteryPluginFactory::name() const
{
    return tr("Battery report");
}

QString BatteryPluginFactory::description() const
{
    return tr("Sends battery charge changes.");
}

QString BatteryPluginFactory::iconUrl() const
{
    return "image://theme/icon-m-battery";
}

} // namespace SailfishConnect

Q_IMPORT_PLUGIN(BatteryPluginFactory)
