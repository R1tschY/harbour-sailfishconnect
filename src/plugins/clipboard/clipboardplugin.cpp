#include "clipboardplugin.h"

#include <QGuiApplication>
#include <QClipboard>

namespace SailfishConnect {

static QString packageType = QStringLiteral("kdeconnect.clipboard");

ClipboardPlugin::ClipboardPlugin(
        Device* device,
        const QString &name,
        const QSet<QString> &outgoingCapabilities)
    : KdeConnectPlugin(device, name, outgoingCapabilities)
{
    QClipboard *clipboard = QGuiApplication::clipboard();

    connect(
        clipboard, &QClipboard::dataChanged,
        this, &ClipboardPlugin::changed);
}

bool ClipboardPlugin::receivePackage(const NetworkPackage &np)
{
    ignoreNextChange_ = true;

    QString content = np.get<QString>(QStringLiteral("content"));
    QGuiApplication::clipboard()->setText(content);
}

void ClipboardPlugin::changed()
{
    if (ignoreNextChange_) {
        ignoreNextChange_ = false;
        return;
    }

    QClipboard *clipboard = QGuiApplication::clipboard();

    NetworkPackage np(packageType, {{"content", clipboard->text()}});
    sendPackage(np);
}

} // namespace SailfishConnect

Q_IMPORT_PLUGIN(ClipboardPluginFactory)
