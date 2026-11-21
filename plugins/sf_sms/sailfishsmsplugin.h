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

#include <QSqlDatabase>
#include <QVariantList>
#include <QVariantMap>

/// Share the SMS history of the device with the peer and send SMS for it.
///
/// Messages are read from the commhistory database (no lock, read only
/// queries) and sent through Telepathy, which is the telephony stack used by
/// Sailfish OS.
class SailfishSmsPlugin : public KdeConnectPlugin
{
    Q_OBJECT

public:
    SailfishSmsPlugin(QObject *parent, const QVariantList &args);
    ~SailfishSmsPlugin() override;

    void connected() override
    {
    }
    void receivePacket(const NetworkPacket &np) override;

private:
    bool ensureDatabase();
    QString databasePath() const;

    void requestConversations();
    void requestConversation(const NetworkPacket &np);
    void sendSms(const NetworkPacket &np);

    static QVariantMap messageToMap(
        qint64 id, qint64 threadId, const QString &address, const QString &body,
        qint64 timestamp, int direction, bool read);

    bool sendTextMessage(const QString &address, const QString &body);

    QSqlDatabase m_database;
};
