/**
 * Copyright 2015 Albert Vaca <albertvaka@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "kdeconnectpluginconfig.h"

#include <QDir>
#include <QSettings>

#include "kdeconnectconfig.h"

struct SailfishConnectPluginConfigPrivate
{
    QDir m_configDir;
    QScopedPointer<QSettings> m_config;
};

SailfishConnectPluginConfig::SailfishConnectPluginConfig(const QString& deviceId, const QString& pluginName)
    : d(new SailfishConnectPluginConfigPrivate())
{
    d->m_configDir = KdeConnectConfig::instance()->pluginConfigDir(deviceId, pluginName);
    QDir().mkpath(d->m_configDir.path());

    d->m_config.reset(new QSettings(d->m_configDir.absoluteFilePath(QStringLiteral("config")), QSettings::IniFormat));
}

SailfishConnectPluginConfig::~SailfishConnectPluginConfig() = default;

QVariant SailfishConnectPluginConfig::get(const QString& key, const QVariant& defaultValue)
{
    d->m_config->sync();
    return d->m_config->value(key, defaultValue);
}

QVariantList SailfishConnectPluginConfig::getList(const QString& key,
                                             const QVariantList& defaultValue)
{
    QVariantList list;
    d->m_config->sync();  // note: need sync() to get recent changes signalled from other process
    int size = d->m_config->beginReadArray(key);
    if (size < 1) {
        d->m_config->endArray();
        return defaultValue;
    }
    for (int i = 0; i < size; ++i) {
        d->m_config->setArrayIndex(i);
        list << d->m_config->value(QStringLiteral("value"));
    }
    d->m_config->endArray();
    return list;
}

void SailfishConnectPluginConfig::set(const QString& key, const QVariant& value)
{
    d->m_config->setValue(key, value);
    d->m_config->sync();
}

void SailfishConnectPluginConfig::setList(const QString& key, const QVariantList& list)
{
    d->m_config->beginWriteArray(key);
    for (int i = 0; i < list.size(); ++i) {
        d->m_config->setArrayIndex(i);
        d->m_config->setValue(QStringLiteral("value"), list.at(i));
    }
    d->m_config->endArray();
    d->m_config->sync();
}

void SailfishConnectPluginConfig::slotConfigChanged()
{
    Q_EMIT configChanged();
}
