#ifndef CLIPBOARDPLUGIN_H
#define CLIPBOARDPLUGIN_H

#include <QObject>
#include <QtPlugin>

#include "../../core/kdeconnectplugin.h"

namespace SailfishConnect {

class ClipboardPlugin : public KdeConnectPlugin
{
    Q_OBJECT
public:
    ClipboardPlugin(
            Device* device,
            const QString &name,
            const QSet<QString> &outgoingCapabilities);

public slots:
    bool receivePackage(const NetworkPackage &np) override;

private:
    void changed();

    bool ignoreNextChange_ = false;
};

class ClipboardPluginFactory :
        public SailfishConnectPluginFactory_<ClipboardPlugin>
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID SailfishConnectPlugin_iid FILE "metadata.json")
    Q_INTERFACES(SailfishConnectPluginFactory)
public:
    using SailfishConnectPluginFactory_<ClipboardPlugin>
        ::SailfishConnectPluginFactory_;
};

} // namespace SailfishConnect

#endif // CLIPBOARDPLUGIN_H
