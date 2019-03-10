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

#include "keyboardlayoutprovider.h"
#include <QFile>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>
#include <sailfishapp.h>

KeyboardLayoutProvider::KeyboardLayoutProvider(QObject *parent) : QObject(parent)
{
    setLayout("german");
}

QString KeyboardLayoutProvider::layout() const
{
    return m_layout;
}

void KeyboardLayoutProvider::setLayout(const QString &layout)
{
    // seems to be the only way to get the files stored
    QFile layoutFile(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/keyboard-layouts/german.json");
    if (!layoutFile.open(QIODevice::ReadOnly)) {
        qDebug() << "unkown layout: " << layout;
        return;
    }

    QJsonDocument layoutJson = QJsonDocument::fromJson(layoutFile.readAll());

    layoutFile.close();

    QJsonArray rowArray = layoutJson.array();
    m_row1 = rowArray[0].toArray().toVariantList();
    m_row2 = rowArray[1].toArray().toVariantList();
    m_row3 = rowArray[2].toArray().toVariantList();
    m_row4 = rowArray[3].toArray().toVariantList();
    m_row5 = rowArray[4].toArray().toVariantList();
    m_row6 = rowArray[5].toArray().toVariantList();

    m_layout = layout;

    emit layoutChanged();
}

QVariantList KeyboardLayoutProvider::row1() const
{
    return m_row1;
}

QVariantList KeyboardLayoutProvider::row2() const
{
    return m_row2;
}

QVariantList KeyboardLayoutProvider::row3() const
{
    return m_row3;
}

QVariantList KeyboardLayoutProvider::row4() const
{
    return m_row4;
}

QVariantList KeyboardLayoutProvider::row5() const
{
    return m_row5;
}

QVariantList KeyboardLayoutProvider::row6() const
{
    return m_row6;
}
