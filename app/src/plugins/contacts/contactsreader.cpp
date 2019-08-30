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
