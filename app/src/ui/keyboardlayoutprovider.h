/*
 * Copyright 2019 Lieven Hey <t.schilling@snafu.de>.
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

#ifndef KEYBOARDLAYOUTPROVIDER_H
#define KEYBOARDLAYOUTPROVIDER_H

#include <QObject>
#include <QList>
#include <QVariant>

class KeyboardLayoutProvider : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString layout READ layout WRITE setLayout NOTIFY layoutChanged)
    Q_PROPERTY(QVariantList row1 READ row1 NOTIFY layoutChanged)
    Q_PROPERTY(QVariantList row2 READ row2 NOTIFY layoutChanged)
    Q_PROPERTY(QVariantList row3 READ row3 NOTIFY layoutChanged)
    Q_PROPERTY(QVariantList row4 READ row4 NOTIFY layoutChanged)
    Q_PROPERTY(QVariantList row5 READ row5 NOTIFY layoutChanged)
    Q_PROPERTY(QVariantList row6 READ row6 NOTIFY layoutChanged)
public:
    explicit KeyboardLayoutProvider(QObject *parent = nullptr);

    QString layout() const;
    void setLayout(const QString &layout);

    Q_INVOKABLE QVariantList layouts();

    QVariantList row1() const;
    QVariantList row2() const;
    QVariantList row3() const;
    QVariantList row4() const;
    QVariantList row5() const;
    QVariantList row6() const;

signals:
    void layoutChanged();

private:
    QString m_layout;
    QVariantList m_row1;
    QVariantList m_row2;
    QVariantList m_row3;
    QVariantList m_row4;
    QVariantList m_row5;
    QVariantList m_row6;
};

#endif // KEYBOARDLAYOUTPROVIDER_H
