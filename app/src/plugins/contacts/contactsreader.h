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

#ifndef CONTACTSREADER_H
#define CONTACTSREADER_H

#include "contactsstore.h"
#include <QProcess>


namespace SailfishConnect {

class ListUIdsJob : public QObject {
    Q_OBJECT
public:
    ListUIdsJob(QObject* parent);

signals:
    void finished(const QString& error, const QStringList& uIds);

private slots:
    void start();

private:
    QProcess m_process;

    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
};


class ContactsReader
{
public:
    ContactsReader();

    static ListUIdsJob *readIds();

    void read();

private:

};

class VCardBuilder
{
public:
    VCardBuilder();

    void addRawProperty(const QString& name, const QString& rawValue);

    QString result();

private:
    QString m_vCard;
};

} // namespace SailfishConnect

#endif // CONTACTSREADER_H
