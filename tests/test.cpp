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
