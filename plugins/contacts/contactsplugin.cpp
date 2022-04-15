/*
 * Copyright 2020 Richard Liebscher <richard.liebscher@gmail.com>.
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

#include <QLoggingCategory>
#include <KPluginFactory>

#include <contactsmanager.h>
#include <vcardbuilder.h>


using namespace SailfishConnect;


K_PLUGIN_CLASS_WITH_JSON(ContactsPlugin, "sailfishconnect_contacts.json")

static Q_LOGGING_CATEGORY(logger, "kdeconnect.plugin.contacts");

static const QString PACKET_TYPE_CONTACTS_REQUEST_ALL_UIDS_TIMESTAMP = QStringLiteral("kdeconnect.contacts.request_all_uids_timestamps");

static const QString PACKET_TYPE_CONTACTS_REQUEST_VCARDS_BY_UIDS = QStringLiteral("kdeconnect.contacts.request_vcards_by_uid");

static const QString PACKAGE_TYPE_CONTACTS_RESPONSE_UIDS_TIMESTAMPS = QStringLiteral("kdeconnect.contacts.response_uids_timestamps");

static const QString PACKET_TYPE_CONTACTS_RESPONSE_VCARDS = QStringLiteral("kdeconnect.contacts.response_vcards");


bool ContactsPlugin::receivePacket(const NetworkPacket& np)
{
    if (np.type() == PACKET_TYPE_CONTACTS_REQUEST_ALL_UIDS_TIMESTAMP) {
        NetworkPacket resultNp(PACKAGE_TYPE_CONTACTS_RESPONSE_UIDS_TIMESTAMPS);

        auto times = ContactsManager::instance().getLastModifiedTimes();
        if (times.isEmpty())
            return true;

        for (auto iter = times.begin(); iter != times.end(); ++iter) {
            resultNp.set(iter.key(), iter.value().toMSecsSinceEpoch());
        }

        resultNp.set(QStringLiteral("uids"), times.keys());
        sendPacket(resultNp);
        return true;
    }

    if (np.type() == PACKET_TYPE_CONTACTS_REQUEST_VCARDS_BY_UIDS) {
        NetworkPacket resultNp(PACKET_TYPE_CONTACTS_RESPONSE_VCARDS);

        auto vcards = ContactsManager::instance().exportVCards(
            np.get<QStringList>("uids"), device()->id());
        if (vcards.isEmpty())
            return true;

        for (auto iter = vcards.begin(); iter != vcards.end(); ++iter) {
            resultNp.set(iter.key(), iter.value());
        }

        resultNp.set(QStringLiteral("uids"), vcards.keys());
        sendPacket(resultNp);
        return true;
    }

    return false;
}

#include "contactsplugin.moc"
