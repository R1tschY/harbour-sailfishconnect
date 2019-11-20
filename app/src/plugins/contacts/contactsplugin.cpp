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

#include <QDateTime>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QSet>
#include <QUrl>
#include <QtGlobal>

#include <appdaemon.h>
#include <helper/contactsmanager.h>
#include <helper/vcardbuilder.h>
#include <sailfishconnect/daemon.h>
#include <sailfishconnect/device.h>
#include <sailfishconnect/helper/cpphelper.h>
#include <sailfishconnect/networkpacket.h>

namespace SailfishConnect {

static Q_LOGGING_CATEGORY(logger, "kdeconnect.plugin.contacts");

QString PACKET_TYPE_CONTACTS_REQUEST_ALL_UIDS_TIMESTAMP = QStringLiteral("kdeconnect.contacts.request_all_uids_timestamps");

QString PACKET_TYPE_CONTACTS_REQUEST_VCARDS_BY_UIDS = QStringLiteral("kdeconnect.contacts.request_vcards_by_uid");

QString PACKAGE_TYPE_CONTACTS_RESPONSE_UIDS_TIMESTAMPS = QStringLiteral("kdeconnect.contacts.response_uids_timestamps");

QString PACKET_TYPE_CONTACTS_RESPONSE_VCARDS = QStringLiteral("kdeconnect.contacts.response_vcards");

ContactsPlugin::ContactsPlugin(
    Device* device, QString name, QSet<QString> outgoingCapabilities)
    : KdeConnectPlugin(device, name, outgoingCapabilities)
{
}

bool ContactsPlugin::receivePacket(const NetworkPacket& np)
{
    if (np.type() == PACKET_TYPE_CONTACTS_REQUEST_ALL_UIDS_TIMESTAMP) {
        NetworkPacket resultNp(PACKAGE_TYPE_CONTACTS_RESPONSE_UIDS_TIMESTAMPS);

        auto times = AppDaemon::instance()->getContacts()->getLastModifiedTimes();
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

        auto vcards = AppDaemon::instance()->getContacts()->exportVCards(np.get<QStringList>("uids"), device()->id());
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
