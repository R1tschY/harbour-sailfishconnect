/*
 * Copyright 2019 Lieven Hey <t.schilling@snafu.de>.
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


#ifndef REMOTEINPUT_H
#define REMOTEINPUT_H

#include <QObject>

#include <sailfishconnect/kdeconnectplugin.h>

namespace SailfishConnect {
class RemoteInputPlugin : public KdeConnectPlugin
{
    Q_OBJECT
public:
    RemoteInputPlugin(Device* device, const QString &name, const QSet<QString> &outgoingCapabilities);

    bool receivePacket(const NetworkPacket &np) override;

    Q_SCRIPTABLE void sendKeyPress(const QString &key, bool shift = false, bool ctrl = false, bool alt = false) const;
};

class RemoteInputPluginFactory: public SailfishConnectPluginFactory_<RemoteInputPlugin>
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID SailfishConnectPlugin_iid FILE "metadata.json")
    Q_INTERFACES(SailfishConnectPluginFactory)
public:
    using SailfishConnectPluginFactory_<RemoteInputPlugin>::SailfishConnectPluginFactory_;

    QString name() const override;
    QString description() const override;
    QString iconUrl() const override;
};
}

#endif // REMOTEINPUT_H
