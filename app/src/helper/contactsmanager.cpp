#include "contactsmanager.h"

#include <QLoggingCategory>
#include <QSqlError>
#include <QSqlQuery>

#include "vcardbuilder.h"

namespace SailfishConnect {

static Q_LOGGING_CATEGORY(logger, "kdeconnect.contactsmanager");

static QString QTCONTACTS_SQLITE_STORE = QStringLiteral(
    "/home/nemo/.local/share/system/Contacts/qtcontacts-sqlite/contacts.db");

ContactsManager::ContactsManager(QObject* parent)
    : QObject(parent)
{
    m_db = QSqlDatabase::addDatabase(
        QStringLiteral("QSQLITE"),
        QStringLiteral("QTCONTACTS_SQLITE_STORE"));
    if (!m_db.isValid()) {
        qCCritical(logger) << "QSQLITE database driver is not available.";
        return;
    }

    m_db.setDatabaseName(QTCONTACTS_SQLITE_STORE);
    if (!m_db.open()) {
        qCCritical(logger) << "Cannot open contacts database"
                           << QTCONTACTS_SQLITE_STORE
                           << ":" << m_db.lastError().text();
        return;
    }
}

QMap<QString, QDateTime> ContactsManager::getLastModifiedTimes()
{
    QMap<QString, QDateTime> result;

    QSqlQuery timeStampQuery(m_db);
    if (!timeStampQuery.exec(
            QStringLiteral("SELECT contactId, modified FROM Contacts"))) {
        qCCritical(logger) << "Getting modified timestamps failed:"
                           << timeStampQuery.lastError().text();
        return result;
    }

    while (timeStampQuery.next()) {
        QString id = QString::number(timeStampQuery.value(0).toInt());
        QDateTime modified = timeStampQuery.value(1).toDateTime();
        result.insert(id, modified);
    }

    return result;
}

QMap<QString, QString> ContactsManager::exportVCards(const QStringList& ids, const QString& deviceId)
{
    QMap<QString, QString> result;
    QSet<QString> requestedIds = ids.toSet();

    QSqlQuery phoneNumbersQuery(m_db);
    if (!phoneNumbersQuery.exec(QStringLiteral(
            "SELECT DISTINCT contactId, phoneNumber FROM PhoneNumbers"))) {
        qCCritical(logger) << "Getting phone numbers failed:"
                           << phoneNumbersQuery.lastError().text();
        return {};
    }

    QMap<QString, QStringList> phoneNumbers;
    while (phoneNumbersQuery.next()) {
        QString id = QString::number(phoneNumbersQuery.value(0).toInt());
        QString phoneNumber = phoneNumbersQuery.value(1).toString();
        if (phoneNumbers.contains(id)) {
            phoneNumbers[id].append(phoneNumber);
        } else {
            phoneNumbers.insert(id, { phoneNumber });
        }
    }

    QSqlQuery dataQuery(m_db);
    if (!dataQuery.exec(QStringLiteral(
            "SELECT contactId, modified, displayLabel, firstName, lastName "
            "FROM Contacts"))) {
        qCCritical(logger) << "Getting contact details failed:"
                           << dataQuery.lastError().text();
        return {};
    }

    while (dataQuery.next()) {
        QString id = QString::number(dataQuery.value(0).toInt());
        if (!requestedIds.contains(id))
            continue;

        QDateTime modified = dataQuery.value(1).toDateTime();
        QString displayLabel = dataQuery.value(2).toString();
        QString firstName = dataQuery.value(3).toString();
        QString lastName = dataQuery.value(4).toString();

        VCardBuilder vcf;
        vcf.addRawProperty(QStringLiteral("FN"), displayLabel);
        for (auto& tel : phoneNumbers.value(id)) {
            vcf.addRawProperty(QStringLiteral("TEL;TYPE=home"), tel);
        }
        vcf.addRawProperty(
            QStringLiteral("N"),
            QStringLiteral("%1;%2;;;").arg(lastName, firstName));
        vcf.addRawProperty(
            QStringLiteral("X-KDECONNECT-ID-DEV-") + deviceId,
            id);
        vcf.addRawProperty(
            QStringLiteral("X-KDECONNECT-TIMESTAMP"),
            QString::number(modified.toMSecsSinceEpoch()));

        result.insert(id, vcf.result());
    }

    return result;
}

QString ContactsManager::lookUpName(const QString& phoneNumber)
{
    QSqlQuery dataQuery(m_db);
    dataQuery.prepare(QStringLiteral(
        "SELECT displayLabel FROM Contacts"
        "JOIN Phonenumbers ON Phonenumbers.contactId == Contacts.contactId"
        "WHERE Phonenumbers.phoneNumber == ?"));
    dataQuery.bindValue(0, phoneNumber);

    if (!dataQuery.exec()) {
        qCCritical(logger) << "Getting contact name failed:"
                           << dataQuery.lastError().text();
        return QString();
    }

    if (dataQuery.next()) {
        return dataQuery.value(0).toString();
    } else {
        qCInfo(logger) << "Getting contact name failed: no entry found";
        return phoneNumber;
    }
}

} // namespace SailfishConnect
