/*
 * Copyright 2015 Holger Kaelberer <holger.k@elberer.de>
 * Copyright 2018 Richard Liebscher <richard.liebscher@gmail.com>.
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

#pragma once

#include <sailfishconnect/device.h>
#include <QIODevice>
#include <QSharedPointer>
#include <QThread>
#include <QHash>
#include <QSet>
#include <dbus/dbus.h>

class KdeConnectPlugin;

namespace SailfishConnect {

struct NotifyingApplication;

class RawDbusError {
public:
    RawDbusError() {
        dbus_error_init(&err);
    }

    RawDbusError(const RawDbusError&) = delete;
    RawDbusError operator=(const RawDbusError&) = delete;
    RawDbusError(RawDbusError&&) = delete;
    RawDbusError operator=(RawDbusError&&) = delete;

    ~RawDbusError() {
        dbus_error_free(&err);
    }

    QString message() {
        return QString::fromUtf8(err.message);
    }

    QString name() {
        return QString::fromUtf8(err.name);
    }

    bool isSet() {
        return dbus_error_is_set(&err);
    }

    explicit operator bool() {
        return isSet();
    }

    DBusError* rawPtr() {
        return &err;
    }

    DBusError* operator&() {
        return &err;
    }

private:
    DBusError err;
};

class NotificationsListenerThread : public QThread {
    Q_OBJECT
public:
    NotificationsListenerThread();
    ~NotificationsListenerThread();

    bool hasLastError() {
        return lastError.isSet();
    }

    QString lastErrorName() {
        return lastError.name();
    }

    QString lastErrorMessage() {
        return lastError.message();
    }

    void quit();

    void handleMessage(DBusMessage *message);

signals:
    void Notify(const QString&, uint, const QString&,
                const QString&, const QString&,
                const QStringList&, const QVariantMap&, int);
protected:
    void run() override;

private:
    QAtomicPointer<DBusConnection> m_connection = nullptr;
    RawDbusError lastError;

    void handleNotifyCall(DBusMessage *message);
};

class IconCacheEntry : public QObject {
public:
private:
    QByteArray pngData;
    QByteArray md5Hash;
};

// TODO: make singleton with shapedpointer/weakpointer
class NotificationsListener : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.Notifications")

public:
    explicit NotificationsListener(KdeConnectPlugin* aPlugin);
    ~NotificationsListener() override;

protected:
    // virtual helper function to make testing possible (QDBusArgument can not
    // be injected without making a DBUS-call):
    virtual bool parseImageDataArgument(const QVariant& argument, int& width,
                                        int& height, int& rowStride, int& bitsPerSample,
                                        int& channels, bool& hasAlpha,
                                        QByteArray& imageData) const;
    QSharedPointer<QIODevice> iconForImageData(const QVariant& argument) const;
    QSharedPointer<QIODevice> iconForIconName(const QString& iconName) const;

private Q_SLOTS:
    void loadApplications();
    void onNotify(const QString&, uint, const QString&,
                  const QString&, const QString&,
                  const QStringList&, const QVariantMap&, int);

private:
    QSharedPointer<QIODevice> pngFromImage();

    KdeConnectPlugin* m_plugin;
    QHash<QString, NotifyingApplication> m_applications;
    NotificationsListenerThread* m_thread;
    //QCache<IconCacheEntry> m_iconCache;
};

} // namespace SailfishConnect
