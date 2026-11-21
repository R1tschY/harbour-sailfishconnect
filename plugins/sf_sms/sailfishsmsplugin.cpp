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

#include "sailfishsmsplugin.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDir>
#include <QLoggingCategory>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>

#include <KPluginFactory>

K_PLUGIN_CLASS_WITH_JSON(SailfishSmsPlugin, "sailfishconnect_sms.json")

Q_LOGGING_CATEGORY(loggerSms, "kdeconnect.plugin.sms")

namespace {

const QString packetTypeSmsRequest = QStringLiteral("kdeconnect.sms.request");
const QString packetTypeSmsRequestConversations =
    QStringLiteral("kdeconnect.sms.request_conversations");
const QString packetTypeSmsRequestConversation =
    QStringLiteral("kdeconnect.sms.request_conversation");
const QString packetTypeSmsMessages = QStringLiteral("kdeconnect.sms.messages");

constexpr int smsType = 2;
constexpr int mmsType = 3;
constexpr int directionIncoming = 1;
constexpr int defaultMessageLimit = 100;
QString normalizeAddress(const QString &address)
{
    QString normalized;
    normalized.reserve(address.size());
    for (const QChar &character : address) {
        if (character.isDigit() || character == QLatin1Char('+')) {
            normalized.append(character);
        }
    }

    // ofono expects international numbers with a leading plus sign.
    if (normalized.startsWith(QStringLiteral("00"))) {
        normalized = QLatin1Char('+') + normalized.mid(2);
    }

    return normalized;
}


} // namespace

SailfishSmsPlugin::SailfishSmsPlugin(QObject *parent, const QVariantList &args)
    : KdeConnectPlugin(parent, args)
{
}

SailfishSmsPlugin::~SailfishSmsPlugin()
{
    const QString connectionName = m_database.connectionName();
    if (m_database.isOpen()) {
        m_database.close();
    }
    m_database = QSqlDatabase();
    if (!connectionName.isEmpty()) {
        QSqlDatabase::removeDatabase(connectionName);
    }
}

QString SailfishSmsPlugin::databasePath() const
{
    return QDir::homePath() + QStringLiteral("/.local/share/commhistory/commhistory.db");
}

bool SailfishSmsPlugin::ensureDatabase()
{
    if (m_database.isOpen()) {
        return true;
    }

    const QString connectionName =
        QStringLiteral("sailfishconnect-sms-%1").arg(device()->id());
    if (!QSqlDatabase::contains(connectionName)) {
        m_database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connectionName);
    } else {
        m_database = QSqlDatabase::database(connectionName, false);
    }
    m_database.setDatabaseName(databasePath());

    if (!m_database.open()) {
        qCWarning(loggerSms) << "Cannot open commhistory database:"
                             << m_database.lastError().text();
        return false;
    }

    return true;
}

QVariantMap SailfishSmsPlugin::messageToMap(qint64 id, qint64 threadId,
                                            const QString &address,
                                            const QString &body,
                                            qint64 timestamp, int direction,
                                            bool read)
{
    QVariantMap message;
    // Android's event flags: 1 = text message.
    message.insert(QStringLiteral("event"), 1);
    message.insert(QStringLiteral("body"), body);
    // The protocol expects milliseconds, commhistory stores seconds.
    message.insert(QStringLiteral("date"), QString::number(timestamp * 1000));
    message.insert(QStringLiteral("type"),
                   direction == directionIncoming ? 1 : 2);
    message.insert(QStringLiteral("thread_id"), threadId);
    message.insert(QStringLiteral("read"), read);
    message.insert(QStringLiteral("_id"), id);

    QVariantMap addressMap;
    addressMap.insert(QStringLiteral("address"), address);
    QVariantList addresses;
    addresses.append(addressMap);
    message.insert(QStringLiteral("addresses"), addresses);

    return message;
}

void SailfishSmsPlugin::requestConversations()
{
    if (!ensureDatabase()) {
        return;
    }

    QSqlQuery query(m_database);
    query.setForwardOnly(true);
    const QString sql = QStringLiteral(
        "SELECT e.id AS id, e.groupId AS threadId, e.remoteUid AS address, "
        "e.freeText AS body, e.endTime AS timestamp, e.isRead AS read, "
        "e.direction AS direction, g.remoteUids AS groupAddress "
        "FROM Events e JOIN Groups g ON g.id = e.groupId "
        "WHERE e.type IN (%1, %2) AND COALESCE(e.isDeleted, 0) = 0 "
        "AND e.id = (SELECT e2.id FROM Events e2 "
        "            WHERE e2.groupId = e.groupId AND e2.type IN (%1, %2) "
        "            AND COALESCE(e2.isDeleted, 0) = 0 "
        "            ORDER BY e2.endTime DESC, e2.id DESC LIMIT 1) "
        "ORDER BY e.endTime DESC")
                            .arg(smsType)
                            .arg(mmsType);

    if (!query.exec(sql)) {
        qCWarning(loggerSms) << "Cannot query conversations:"
                             << query.lastError().text();
        return;
    }

    QVariantList messages;
    while (query.next()) {
        QString address = query.value(QStringLiteral("address")).toString();
        if (address.isEmpty()) {
            address = query.value(QStringLiteral("groupAddress")).toString();
        }
        messages.append(messageToMap(
            query.value(QStringLiteral("id")).toLongLong(),
            query.value(QStringLiteral("threadId")).toLongLong(), address,
            query.value(QStringLiteral("body")).toString(),
            query.value(QStringLiteral("timestamp")).toLongLong(),
            query.value(QStringLiteral("direction")).toInt(),
            query.value(QStringLiteral("read")).toInt() != 0));
    }

    NetworkPacket reply(packetTypeSmsMessages);
    reply.set(QStringLiteral("version"), 2);
    reply.set(QStringLiteral("messages"), messages);
    sendPacket(reply);
}

void SailfishSmsPlugin::requestConversation(const NetworkPacket &np)
{
    if (!ensureDatabase()) {
        return;
    }

    const qint64 threadId = np.get<qint64>(QStringLiteral("threadID"), 0);
    const qint64 rangeStartTimestamp =
        np.get<qint64>(QStringLiteral("rangeStartTimestamp"), 0);
    qint64 limit = np.get<qint64>(QStringLiteral("numberToRequest"), 0);
    if (limit <= 0) {
        limit = defaultMessageLimit;
    }

    QSqlQuery query(m_database);
    query.setForwardOnly(true);
    query.prepare(QStringLiteral(
        "SELECT id AS id, groupId AS threadId, remoteUid AS address, "
        "freeText AS body, endTime AS timestamp, isRead AS read, "
        "direction AS direction "
        "FROM Events "
        "WHERE groupId = :threadId AND type IN (:smsType, :mmsType) "
        "AND COALESCE(isDeleted, 0) = 0 AND (:rangeStartA = 0 OR endTime * 1000 < :rangeStartB) "
        "ORDER BY endTime DESC, id DESC LIMIT :limit"));
    query.bindValue(QStringLiteral(":threadId"), threadId);
    query.bindValue(QStringLiteral(":smsType"), smsType);
    query.bindValue(QStringLiteral(":mmsType"), mmsType);
    query.bindValue(QStringLiteral(":rangeStartA"), rangeStartTimestamp);
    query.bindValue(QStringLiteral(":rangeStartB"), rangeStartTimestamp);
    query.bindValue(QStringLiteral(":limit"), limit);

    if (!query.exec()) {
        qCWarning(loggerSms) << "Cannot query conversation:" << query.lastError().text();
        return;
    }

    QVariantList messages;
    while (query.next()) {
        messages.prepend(messageToMap(
            query.value(QStringLiteral("id")).toLongLong(),
            query.value(QStringLiteral("threadId")).toLongLong(),
            query.value(QStringLiteral("address")).toString(),
            query.value(QStringLiteral("body")).toString(),
            query.value(QStringLiteral("timestamp")).toLongLong(),
            query.value(QStringLiteral("direction")).toInt(),
            query.value(QStringLiteral("read")).toInt() != 0));
    }

    NetworkPacket reply(packetTypeSmsMessages);
    reply.set(QStringLiteral("version"), 2);
    reply.set(QStringLiteral("messages"), messages);
    sendPacket(reply);
}

void SailfishSmsPlugin::sendSms(const NetworkPacket &np)
{
    QStringList numbers;
    const QVariantList addresses = np.get<QVariantList>(QStringLiteral("addresses"));
    for (const QVariant &address : addresses) {
        const QString number = address.toMap().value(QStringLiteral("address")).toString();
        if (!number.isEmpty()) {
            numbers.append(number);
        }
    }

    const QString body = np.get<QString>(
        QStringLiteral("messageBody"), np.get<QString>(QStringLiteral("message_body")));
    if (numbers.isEmpty() || body.isEmpty()) {
        qCWarning(loggerSms) << "Send request without address or body";
        return;
    }

    for (const QString &number : numbers) {
        if (!sendTextMessage(normalizeAddress(number), body)) {
            qCWarning(loggerSms) << "Cannot send SMS to" << number;
        }
    }
}

bool SailfishSmsPlugin::sendTextMessage(const QString &address,
                                        const QString &body)
{
    QDBusConnection bus = QDBusConnection::systemBus();
    if (!bus.isConnected()) {
        qCWarning(loggerSms) << "Cannot connect to the system bus";
        return false;
    }

    // The SIM can be in either slot, ofono exposes one modem object per slot.
    const QStringList modems{QStringLiteral("/ril_0"), QStringLiteral("/ril_1")};
    for (const QString &modem : modems) {
        QDBusMessage check = QDBusMessage::createMethodCall(
            QStringLiteral("org.ofono"), modem,
            QStringLiteral("org.ofono.MessageManager"), QStringLiteral("GetProperties"));
        const QDBusMessage checkReply = bus.call(check, QDBus::Block, 3000);
        if (checkReply.type() == QDBusMessage::ErrorMessage) {
            continue;
        }

        QDBusMessage send = QDBusMessage::createMethodCall(
            QStringLiteral("org.ofono"), modem,
            QStringLiteral("org.ofono.MessageManager"), QStringLiteral("SendMessage"));
        send << address << body;

        const QDBusMessage sendReply = bus.call(send, QDBus::Block, 30000);
        if (sendReply.type() != QDBusMessage::ErrorMessage) {
            qCDebug(loggerSms) << "SMS queued on" << modem << "for" << address;
            return true;
        }

        qCWarning(loggerSms) << "ofono refused the SMS on" << modem << ":"
                             << sendReply.errorMessage();
    }

    return false;
}

void SailfishSmsPlugin::receivePacket(const NetworkPacket &np)
{
    if (np.type() == packetTypeSmsRequest) {
        sendSms(np);
    } else if (np.type() == packetTypeSmsRequestConversations) {
        requestConversations();
    } else if (np.type() == packetTypeSmsRequestConversation) {
        requestConversation(np);
    } else {
        qCDebug(loggerSms) << "Ignoring unsupported SMS packet:" << np.type();
    }
}

#include "sailfishsmsplugin.moc"
