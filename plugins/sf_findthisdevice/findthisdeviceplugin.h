/*
 * Copyright 2026 Manuel FLURY <zx97@users.noreply.github.com>.
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

#include <core/kdeconnectplugin.h>

#include <QDBusInterface>

class Notification;
class QTimer;

/// Rings the device when the peer asks for it ("Find my phone").
///
/// The ringtone is played through ngfd, the non-graphic feedback daemon of
/// Sailfish OS, so it uses the ringtone and the volume selected by the user
/// and keeps playing with the screen locked.
class FindThisDevicePlugin : public KdeConnectPlugin
{
    Q_OBJECT

public:
    FindThisDevicePlugin(QObject *parent, const QVariantList &args);
    ~FindThisDevicePlugin() override;

    void connected() override
    {
    }
    void receivePacket(const NetworkPacket &np) override;

public slots:
    /// Stops the ringtone and hides the "find my phone" page.
    Q_INVOKABLE void stopRinging();

    /// Whether the ringtone is currently playing.
    Q_INVOKABLE bool isRinging() const
    {
        return m_eventId != 0;
    }

private:
    void startRinging();
    void notifyUi(const QString &method);

    QDBusInterface *m_ngfd = nullptr;
    QTimer *m_timeout = nullptr;
    Notification *m_notification = nullptr;
    uint m_eventId = 0;
};
