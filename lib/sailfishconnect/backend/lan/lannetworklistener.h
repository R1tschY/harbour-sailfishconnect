#ifndef LANNETWORKLISTENER_H
#define LANNETWORKLISTENER_H

#include <QNetworkConfigurationManager>
#include <QObject>
#include <QSet>
#include <QString>

class QNetworkConfiguration;

namespace SailfishConnect {

class LanNetworkListener : public QObject
{
    Q_OBJECT
public:
    explicit LanNetworkListener(QObject *parent = nullptr);

    QNetworkConfigurationManager& networkManager() { return m_networkManager; }

signals:
    void networkChanged();

private:
    void onNetworkConfigurationChanged(const QNetworkConfiguration& config);
    void onNetworkConfigurationRemoved(const QNetworkConfiguration& config);
    void printConfig(const QNetworkConfiguration& config);

    QNetworkConfigurationManager m_networkManager;

    QSet<QString> m_activeConfigurations;
};

} // namespace SailfishConnect

#endif // LANNETWORKLISTENER_H
