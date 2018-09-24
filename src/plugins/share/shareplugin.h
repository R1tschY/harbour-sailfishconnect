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

#pragma once

#include <QObject>
#include <QtPlugin>
#include <QClipboard>

#include <core/kdeconnectplugin.h>

namespace SailfishConnect {

class SharePlugin : public KdeConnectPlugin
{
    Q_OBJECT
public:
    SharePlugin(
            Device* device,
            const QString &name,
            const QSet<QString> &outgoingCapabilities);


    QString incomingPath() const;

public slots:
    bool receivePackage(const NetworkPackage &np) override;
    Q_SCRIPTABLE void share(const QUrl &url);

signals:
    void received(const QUrl& url);

private:
    void finishedFileTransfer();
};

class SharePluginFactory :
        public SailfishConnectPluginFactory_<SharePlugin>
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID SailfishConnectPlugin_iid FILE "metadata.json")
    Q_INTERFACES(SailfishConnectPluginFactory)
public:
    using SailfishConnectPluginFactory_<SharePlugin>
        ::SailfishConnectPluginFactory_;

    QString name() const override;
    QString description() const override;
    QString iconUrl() const override;

    void registerTypes() override;
};

} // namespace SailfishConnect
