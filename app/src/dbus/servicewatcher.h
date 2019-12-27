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

#pragma once

#include <QObject>

class QDBusConnectionInterface;

namespace SailfishConnect {

class DBusServiceWatcher : public QObject {
    Q_OBJECT
  
    Q_PROPERTY(QString service READ service WRITE setService NOTIFY serviceChanged)
    Q_PROPERTY(bool registered READ registered NOTIFY registeredChanged)
public:
    DBusServiceWatcher(QObject* parent = nullptr);

    QString service() const {
        return m_service;
    }

    void setService(const QString& value);

    bool registered() const {
        return m_registered;
    }

signals:
    void registeredChanged();
    void serviceChanged();
    
private:
    QDBusConnectionInterface* m_interface = nullptr;

    bool m_registered = false;
    QString m_service;
    
    void setRegistered(bool value) {
        if (m_registered != value) {
            m_registered = value;
            Q_EMIT registeredChanged();
        }
    }

    void onServiceRegistered(const QString &service);
    void onServiceUnregistered(const QString &service);
};

} // namespace SailfishConnect
