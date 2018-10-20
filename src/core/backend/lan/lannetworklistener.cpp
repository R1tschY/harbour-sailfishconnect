#include "lannetworklistener.h"

#include <QNetworkConfiguration>
#include <utils/cpphelper.h>
#include <core/corelogging.h>

namespace SailfishConnect {

LanNetworkListener::LanNetworkListener(QObject *parent) : QObject(parent)
{
    // Detect when a network interface changes status, so we announce ourelves
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

    qDebug(coreLogger) << "Discovered network configurations:";
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
        qDebug(coreLogger) << "New active network configuration:";
        printConfig(config);

        m_activeConfigurations.insert(id);
    } else {
        if (m_activeConfigurations.contains(id)) {
            qDebug(coreLogger) << "Inactivated network configuration:";
            printConfig(config);
            m_activeConfigurations.remove(id);
        }
    }

    int newActiveConfigs = m_activeConfigurations.size();
    if (newActiveConfigs > activeConfigs) {
        networkChanged();
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
    qDebug(coreLogger) << " " << config.name();
    qDebug(coreLogger) << "    id:" << config.identifier();
    qDebug(coreLogger) << "    bearer type:" << config.bearerTypeName();
    qDebug(coreLogger) << "    purpose:" << config.purpose();
    qDebug(coreLogger) << "    state:" << config.state();
}

} // namespace SailfishConnect
