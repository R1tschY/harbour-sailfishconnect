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
