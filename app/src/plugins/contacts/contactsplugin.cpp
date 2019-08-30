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

#include "contactsplugin.h"

#include <QtGlobal>
#include <QLoggingCategory>
#include <QSqlError>
#include <QSqlQuery>
#include <QDateTime>
#include <QSet>
#include <QUrl>
#include <QFileInfo>

#include <contextproperty.h>
#include <sailfishconnect/networkpacket.h>
#include <sailfishconnect/device.h>
#include <sailfishconnect/helper/cpphelper.h>
#include "contactsreader.h"

namespace SailfishConnect {

static Q_LOGGING_CATEGORY(logger, "kdeconnect.plugin.contacts")

QString PACKET_TYPE_CONTACTS_REQUEST_ALL_UIDS_TIMESTAMP =
        QStringLiteral("kdeconnect.contacts.request_all_uids_timestamps");

QString PACKET_TYPE_CONTACTS_REQUEST_VCARDS_BY_UIDS =
        QStringLiteral("kdeconnect.contacts.request_vcards_by_uid");

QString PACKAGE_TYPE_CONTACTS_RESPONSE_UIDS_TIMESTAMPS =
        QStringLiteral("kdeconnect.contacts.response_uids_timestamps");

QString PACKET_TYPE_CONTACTS_RESPONSE_VCARDS =
        QStringLiteral("kdeconnect.contacts.response_vcards");

QString QTCONTACTS_SQLITE_STORE =
        QStringLiteral("/home/nemo/.local/share/system/Contacts"
                       "/qtcontacts-sqlite/contacts.db");


ContactsPlugin::ContactsPlugin(
        Device *device, QString name, QSet<QString> outgoingCapabilities)
    : KdeConnectPlugin(device, name, outgoingCapabilities)
{
    m_db = QSqlDatabase::addDatabase(
                QStringLiteral("QSQLITE"),
                QStringLiteral("QTCONTACTS_SQLITE_STORE"));
    if (!m_db.isValid()) {
      qCCritical(logger) << "QSQLITE database driver is not available.";
      return;
    }

    m_db.setDatabaseName(QTCONTACTS_SQLITE_STORE);
    if (!m_db.open())
    {
      qCCritical(logger) << "Cannot open contacts database"
                         << QTCONTACTS_SQLITE_STORE
                         << ":" << m_db.lastError().text();
      return;
    }
}

bool ContactsPlugin::receivePacket(const NetworkPacket &np)
{
    if (np.type() == PACKET_TYPE_CONTACTS_REQUEST_ALL_UIDS_TIMESTAMP) {
        QSqlQuery timeStampQuery(m_db);
        if (!timeStampQuery.exec(
          QStringLiteral("SELECT contactId, modified FROM Contacts")))
        {
          qCCritical(logger) << "Getting modified timestamps failed:"
            << timeStampQuery.lastError().text();
          return true;
        }

        QStringList ids;
        NetworkPacket resultNp(PACKAGE_TYPE_CONTACTS_RESPONSE_UIDS_TIMESTAMPS);
        while (timeStampQuery.next())
        {
          QString id = QString::number(timeStampQuery.value(0).toInt());
          QDateTime modified = timeStampQuery.value(1).toDateTime();

          ids.append(id);
          resultNp.set(id, modified.toMSecsSinceEpoch());
        }

        resultNp.set(QStringLiteral("uids"), ids);
        sendPacket(resultNp);
        return true;
    }

    if (np.type() == PACKET_TYPE_CONTACTS_REQUEST_VCARDS_BY_UIDS) {
        QSet<QString> requestedIds = np.get<QStringList>("uids").toSet();

        QSqlQuery phoneNumbersQuery(m_db);
        if (!phoneNumbersQuery.exec(QStringLiteral(
            "SELECT DISTINCT contactId, phoneNumber FROM PhoneNumbers")))
        {
          qCCritical(logger) << "Getting phone numbers failed:"
            << phoneNumbersQuery.lastError().text();
          return true;
        }

        QMap<QString, QStringList> phoneNumbers;
        while (phoneNumbersQuery.next())
        {
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
            "FROM Contacts")))
        {
          qCCritical(logger) << "Getting contact details failed:"
            << dataQuery.lastError().text();
          return true;
        }

        QStringList ids;
        NetworkPacket resultNp(PACKET_TYPE_CONTACTS_RESPONSE_VCARDS);
        while (dataQuery.next())
        {
          QString id = QString::number(dataQuery.value(0).toInt());
          if (!requestedIds.contains(id))
              continue;

          QDateTime modified = dataQuery.value(1).toDateTime();
          QString displayLabel = dataQuery.value(2).toString();
          QString firstName = dataQuery.value(3).toString();
          QString lastName = dataQuery.value(4).toString();
          QString phoneNumber = dataQuery.value(5).toString();

          VCardBuilder vcf;
          vcf.addRawProperty(QStringLiteral("FN"), displayLabel);
          for (auto& tel : phoneNumbers.value(id)) {
              vcf.addRawProperty(QStringLiteral("TEL;TYPE=home"), tel);
          }
          vcf.addRawProperty(
            QStringLiteral("N"),
            QStringLiteral("%1;%2;;;").arg(lastName, firstName));
          vcf.addRawProperty(
            QStringLiteral("X-KDECONNECT-ID-DEV-") + device()->id(),
            id);
          vcf.addRawProperty(
            QStringLiteral("X-KDECONNECT-TIMESTAMP"),
            QString::number(modified.toMSecsSinceEpoch()));

          ids.append(id);
          resultNp.set(id, vcf.result());
        }

        resultNp.set(QStringLiteral("uids"), ids);
        sendPacket(resultNp);
        return true;
    }

    return false;
}

QString ContactsPluginFactory::name() const
{
    return tr("Contacts");
}

QString ContactsPluginFactory::description() const
{
    return tr("Share contacts.");
}

QString ContactsPluginFactory::iconUrl() const
{
    return "image://theme/icon-m-contact";
}

} // namespace SailfishConnect

Q_IMPORT_PLUGIN(ContactsPluginFactory)
