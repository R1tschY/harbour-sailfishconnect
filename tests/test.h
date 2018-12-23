#ifndef TEST_H
#define TEST_H

#include <QtCore>

#include <gtest/gtest.h>

QT_BEGIN_NAMESPACE
inline void PrintTo(const QString& str, ::std::ostream *os)
{
    *os << '"' << qPrintable(str) << '"';
}
QT_END_NAMESPACE

#endif // TEST_H
