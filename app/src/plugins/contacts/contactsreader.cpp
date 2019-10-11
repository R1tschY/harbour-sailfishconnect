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

#include "contactsreader.h"

#include <QMetaObject>
#include <QRegularExpression>
#include <sailfishconnect/helper/cpphelper.h>

namespace SailfishConnect {


ListUIdsJob::ListUIdsJob(QObject *parent)
    : QObject(parent)
    , m_process(this)
{
    QMetaObject::invokeMethod(this, "start", Qt::QueuedConnection);
    connect(&m_process,
            Overload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            &ListUIdsJob::onProcessFinished);
}

void ListUIdsJob::start()
{
    m_process.start("contacts-tool", {"list"});
}

void ListUIdsJob::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
//    if (exitStatus != QProcess::NormalExit || exitCode != 0) {
//        // TODO: check for QProcess::error()
//        emit finished(QStringLiteral("Crashed"), QStringList());
//        return;
//    }


//    QString stdout = QString::fromUtf8(m_process.readAllStandardOutput());

//    QStringList result;
//    auto iter = readListRe().globalMatch(stdout);
//    while (iter.hasNext()) {
//        QRegularExpressionMatch match = iter.next();
//        result.append(match.captured(1));
//    }

//    emit finished(QString(), result);
}

ContactsReader::ContactsReader()
{

}

ListUIdsJob *ContactsReader::readIds()
{
    return new ListUIdsJob(nullptr);
}

VCardBuilder::VCardBuilder()
{
    m_vCard.append(QStringLiteral("BEGIN:VCARD\nVERSION:4.0\n"));
}

void VCardBuilder::addRawProperty(const QString &name, const QString &rawValue)
{
    m_vCard.append(name % QChar(':') % rawValue % QChar('\n'));
}

QString VCardBuilder::result()
{
    return m_vCard + QStringLiteral("END:VCARD\n");
}

} // namespace SailfishConnect
