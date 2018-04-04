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
