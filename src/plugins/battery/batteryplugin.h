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

#ifndef BATTERYPLUGIN_H
#define BATTERYPLUGIN_H

#include <QObject>
#include <QtPlugin>
#include <QTimer>

#include "../../core/kdeconnectplugin.h"

class ContextProperty;

namespace SailfishConnect {

class BatteryPlugin : public KdeConnectPlugin
{
    Q_OBJECT
public:
    enum ThresholdBatteryEvent {
        ThresholdNone       = 0,
        ThresholdBatteryLow = 1
    };

    BatteryPlugin(
            Device* device,
            QString name,
            QSet<QString> outgoingCapabilities);

public slots:
    bool receivePackage(const NetworkPackage &np) override;
    void connected() override;

private:
    NetworkPackage batteryPackage_;
    ContextProperty* chargePercentage_;
    ContextProperty* isCharging_;
    ContextProperty* lowBattery_;
    QTimer debounceTimer_;

    void sendStatus();
    void setCheckedStatus(
            const QString& name,
            const QVariant& value,
            int min,
            int max);
};


class BatteryPluginFactory :
        public SailfishConnectPluginFactory_<BatteryPlugin>
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID SailfishConnectPlugin_iid FILE "metadata.json")
    Q_INTERFACES(SailfishConnectPluginFactory)
public:
    using FactoryBaseType::FactoryBaseType;
};

} // namespace SailfishConnect

#endif // BATTERYPLUGIN_H
