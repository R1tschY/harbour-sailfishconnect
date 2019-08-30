#ifndef CONTACTSSTORE_H
#define CONTACTSSTORE_H

#include <QObject>

namespace SailfishConnect {

struct Contact {
    QString uId;
    QString displayName;
    QStringList tel;
};

class ContactsStore : public QObject
{
    Q_OBJECT
public:
    explicit ContactsStore(QObject *parent = nullptr);

signals:

public slots:
};

} // namespace SailfishConnect

#endif // CONTACTSSTORE_H
