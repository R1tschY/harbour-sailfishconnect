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
