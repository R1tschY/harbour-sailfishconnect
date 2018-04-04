#ifndef TELEPATHYPLUGIN_H
#define TELEPATHYPLUGIN_H

#include <QObject>
#include <QtPlugin>

#include "../../core/kdeconnectplugin.h"

class QDBusPendingCallWatcher;
class QDBusInterface;
class QDBusError;

namespace SailfishConnect {

class TelepathyPlugin : public KdeConnectPlugin
{
    Q_OBJECT
public:
    TelepathyPlugin(
            Device* device,
            const QString &name,
            const QSet<QString> &outgoingCapabilities);

public slots:
    bool receivePackage(const NetworkPackage &np) override;

private slots:
    void startSmsSuccess();
    void startSmsError(const QDBusError& error);

private:
    QDBusInterface* m_qmlmessages;
};


class TelepathyPluginFactory :
        public SailfishConnectPluginFactory_<TelepathyPlugin>
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

#endif // TELEPATHYPLUGIN_H
