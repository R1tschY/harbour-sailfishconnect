#include "test.h"

#include <QDebug>
#include <QString>


QT_BEGIN_NAMESPACE

void PrintTo(const QString& str, ::std::ostream *os)
{
    *os << '"' << qPrintable(str) << '"';
}

void PrintTo(const QVariant& x, ::std::ostream *os)
{
    QString out;
    QDebug(&out).nospace() << x;
    *os << qPrintable(out);
}

QT_END_NAMESPACE

QList<QList<QVariant>> toVVList(
        std::initializer_list<std::initializer_list<QVariant>> x)
{
    QList<QList<QVariant>> result;
    result.reserve(x.size());
    for (auto l : x) {
        result.append(QList<QVariant>(l));
    }
    return result;
}
