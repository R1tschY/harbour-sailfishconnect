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
#include <QTimer>

#include <core/kdeconnectplugin.h>

class ContextProperty;

class BatteryReportPlugin : public KdeConnectPlugin
{
    Q_OBJECT
public:
    enum ThresholdBatteryEvent {
        ThresholdNone       = 0,
        ThresholdBatteryLow = 1
    };

    BatteryReportPlugin(QObject* parent, const QVariantList& args);

    bool receivePacket(const NetworkPacket &np) override;
    void connected() override;

private:
    ContextProperty* chargePercentage_;
    ContextProperty* isCharging_;
    ContextProperty* lowBattery_;
    QVariantMap m_state;
    QTimer debounceTimer_;

    void sendStatus();
    void setCheckedStatus(
            const QString& name,
            const QVariant& value,
            int min,
            int max);    
};

#endif // BATTERYPLUGIN_H
