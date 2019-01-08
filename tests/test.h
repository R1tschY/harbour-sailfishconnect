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

#ifndef TEST_H
#define TEST_H

#include <QtCore>
#include <QList>
#include <initializer_list>
#include <tuple>

#include <gtest/gtest.h>

QT_BEGIN_NAMESPACE
void PrintTo(const QString& str, ::std::ostream *os);
void PrintTo(const QVariant& x, ::std::ostream *os);
QT_END_NAMESPACE


QList<QList<QVariant>> toVVList(
        std::initializer_list<std::initializer_list<QVariant>> x);


#endif // TEST_H
