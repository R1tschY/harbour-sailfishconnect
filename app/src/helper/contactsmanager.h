/*
 * Copyright 2019 Richard Liebscher <richard.liebscher@gmail.com>.
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

#ifndef CONTACTSMANAGER_H
#define CONTACTSMANAGER_H

#include <QDateTime>
#include <QMap>
#include <QObject>
#include <QSqlDatabase>

namespace SailfishConnect {

class ContactsManager : public QObject {
    Q_OBJECT
public:
    explicit ContactsManager(QObject* parent = nullptr);

    QMap<QString, QDateTime> getLastModifiedTimes();
    QMap<QString, QString> exportVCards(const QStringList& requestedIds, const QString& deviceId);
    QString lookUpName(const QString& phoneNumber);

private:
    QSqlDatabase m_db;
};

} // namespace SailfishConnect

#endif // CONTACTSMANAGER_H
