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

#ifndef APPDAEMON_H
#define APPDAEMON_H

#include <daemon.h>

#include <QEventLoopLocker>
#include <QSet>
#include <backgroundactivity.h>

class QQmlEngine;
class QQmlImageProviderBase;
class Device;
class Notification;

namespace SailfishConnect {

class JobManager;
class ContactsManager;

class AppDaemon : public Daemon
{
    Q_OBJECT  
    Q_CLASSINFO("D-Bus Interface", "org.kde.kdeconnect.daemon")

public:
    AppDaemon(QObject* parent = nullptr);

    void askPairingConfirmation(Device* device) override;
    void reportError(const QString & title, const QString & description) override;
    void quit() override;
    void sendSimpleNotification(const QString&, const QString&, const QString&, const QString&) override;
    KJobTrackerInterface* jobTracker() override;

    QQmlImageProviderBase* imageProvider(const QString& providerId) const;

    QQmlEngine* qmlEngine() const { return m_qmlEngine; }
    void setQmlEngine(QQmlEngine* qmlEngine);

    static AppDaemon* instance();

private:
    QEventLoopLocker m_eventLoopLock;
    
    BackgroundActivity m_backgroundActivity;

    QQmlEngine* m_qmlEngine = nullptr;
    JobManager* m_jobmanager = nullptr;
    ContactsManager* m_contacts = nullptr;

    void onDeviceVisibilityChanged();
    void onWakeUp();
    void onDeviceAdded(const QString& deviceId);
    void onPairingError(const QString& deviceId, const QString& error);

    Notification* createNotification();
};

} // namespace SailfishConnect

#endif // APPDAEMON_H
