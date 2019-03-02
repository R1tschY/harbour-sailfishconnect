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

#ifndef MANAGER_INTERFACE_H
#define MANAGER_INTERFACE_H

#include <tuple>

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

namespace SailfishConnect {
namespace Ofono {

typedef std::tuple<QDBusObjectPath, QVariantMap> ObjectProperties;
typedef QList<ObjectProperties> ObjectPropertiesArray;


/*
 * Proxy class for interface org.ofono.VoiceCall
 */
class VoiceCall : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "org.ofono.VoiceCall"; }

public:
    VoiceCall(const QString path, QObject *parent = 0);

    QDBusPendingReply<QVariantMap> GetProperties();

Q_SIGNALS:
    void DisconnectReason(const QString& reason);
    void PropertyChanged(const QString& name, const QDBusVariant& value);
};


/*
 * Proxy class for interface org.ofono.VoiceCallManager
 */
class VoiceCallManager : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "org.ofono.VoiceCallManager"; }

public:
    VoiceCallManager(const QString path, QObject *parent = 0);

    QDBusPendingReply<ObjectPropertiesArray> GetCalls();

Q_SIGNALS:
    void CallAdded(const QDBusObjectPath &path, const QVariantMap &properties);
    void CallRemoved(const QDBusObjectPath &path);
};


/*
 * Proxy class for interface org.ofono.Manager
 */
class Manager : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "org.ofono.Manager"; }

public:
    Manager(QObject *parent = 0);

    QDBusPendingReply<ObjectPropertiesArray> GetModems();

Q_SIGNALS:
    void ModemAdded(const QDBusObjectPath &path, const QVariantMap &properties);
    void ModemRemoved(const QDBusObjectPath &path);
};


void registerTypes();

} // namespace Ofono
} // namespace SailfishConnect

Q_DECLARE_METATYPE(SailfishConnect::Ofono::ObjectProperties);
Q_DECLARE_METATYPE(SailfishConnect::Ofono::ObjectPropertiesArray);

#endif
