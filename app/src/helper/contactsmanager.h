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
