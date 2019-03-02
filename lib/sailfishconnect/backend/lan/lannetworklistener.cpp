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

#include "lannetworklistener.h"

#include <QNetworkConfiguration>
#include <sailfishconnect/helper/cpphelper.h>
#include <sailfishconnect/corelogging.h>

namespace SailfishConnect {

LanNetworkListener::LanNetworkListener(QObject *parent) : QObject(parent)
{
    // Detect when a network interface changes status, so we announce ourselves
    // in the new network
    connect(
        &m_networkManager, &QNetworkConfigurationManager::configurationChanged,
        this, &LanNetworkListener::onNetworkConfigurationChanged);
    connect(
        &m_networkManager, &QNetworkConfigurationManager::configurationAdded,
        this, &LanNetworkListener::onNetworkConfigurationChanged);
//    connect(
//        &m_networkManager, &QNetworkConfigurationManager::configurationRemoved,
//        this, &LanNetworkListener::onNetworkConfigurationRemoved);

    qCDebug(coreLogger) << "Discovered network configurations:";
    auto allConfigs = m_networkManager.allConfigurations(
                QNetworkConfiguration::Active);
    for (auto& config : asConst(allConfigs)) {
        m_activeConfigurations.insert(config.identifier());
        printConfig(config);
    }
}

void LanNetworkListener::onNetworkConfigurationChanged(
        const QNetworkConfiguration &config)
{
    int activeConfigs = m_activeConfigurations.size();
    QString id = config.identifier();

    if (config.state().testFlag(QNetworkConfiguration::Active)) {
        m_activeConfigurations.insert(id);
#ifndef QT_NO_DEBUG_OUTPUT
        if (!m_activeConfigurations.contains(id)) {
            qCDebug(coreLogger) << "New active network configuration:";
            printConfig(config);
        }
#endif
    } else {
        m_activeConfigurations.remove(id);
#ifndef QT_NO_DEBUG_OUTPUT
        if (m_activeConfigurations.contains(id)) {
            qCDebug(coreLogger) << "Inactivated network configuration:";
            printConfig(config);
        }
#endif
    }

    int newActiveConfigs = m_activeConfigurations.size();
    if (newActiveConfigs > activeConfigs) {
        Q_EMIT networkChanged();
    }
}

void LanNetworkListener::onNetworkConfigurationRemoved(
        const QNetworkConfiguration &config)
{
    m_activeConfigurations.remove(config.identifier());

    // TODO: disconnect devices?
}

void LanNetworkListener::printConfig(const QNetworkConfiguration &config)
{
    qCDebug(coreLogger) << " " << config.name();
    qCDebug(coreLogger) << "    id:" << config.identifier();
    qCDebug(coreLogger) << "    bearer type:" << config.bearerTypeName();
    qCDebug(coreLogger) << "    purpose:" << config.purpose();
    qCDebug(coreLogger) << "    state:" << config.state();
}

} // namespace SailfishConnect
