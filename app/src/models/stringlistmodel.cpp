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

#include "stringlistmodel.h"

#include <QIcon>
#include <QLoggingCategory>
#include <cstdint>
#include <QUrl>

#include <daemon.h>
#include <device.h>
#include <qtcompat_p.h>

namespace SailfishConnect {

StringListModel::StringListModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(this, &QAbstractItemModel::rowsRemoved, this, &StringListModel::stringListChanged);
    connect(this, &QAbstractItemModel::rowsInserted, this, &StringListModel::stringListChanged);
    connect(this, &QAbstractItemModel::rowsMoved, this, &StringListModel::stringListChanged);
    connect(this, &QAbstractItemModel::modelReset, this, &StringListModel::stringListChanged);
    connect(this, &QAbstractItemModel::dataChanged, this, &StringListModel::stringListChanged);
}

QHash<int, QByteArray> StringListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "display");
    roles.insert(Qt::EditRole, "edit");
    return roles;
}

int StringListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_stringlist.length();
}

QVariant StringListModel::data(const QModelIndex& index, int role) const
{
    const int row = index.row();
    if (row < 0 || row >= m_stringlist.length())
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return m_stringlist[row];
    }

    return QVariant();
}

Qt::ItemFlags StringListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return QAbstractListModel::flags(index);

    return QAbstractListModel::flags(index) | Qt::ItemIsEditable;
}

bool StringListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    const int row = index.row();
    if (row < 0 || row >= m_stringlist.length())
        return false;

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        QString newValue = value.toString();
        if (newValue != m_stringlist[row]) {
            m_stringlist.replace(row, newValue);
            Q_EMIT dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });
        }
        return true;
    }

    return false;
}

void StringListModel::append(const QString &value)
{
    beginInsertRows(QModelIndex(), m_stringlist.length(), m_stringlist.length());
    m_stringlist.append(value);
    endInsertRows();
}

bool StringListModel::removeAt(int index)
{
    if (index < 0 || index >= m_stringlist.length())
        return false;

    beginRemoveRows(QModelIndex(), index, index);
    m_stringlist.removeAt(index);
    endRemoveRows();
    return true;
}

bool StringListModel::removeOne(const QString &value)
{
    int index = m_stringlist.indexOf(value);
    if (index < 0)
        return false;

    beginRemoveRows(QModelIndex(), index, index);
    m_stringlist.removeAt(index);
    endRemoveRows();
    return true;
}

void StringListModel::setStringList(const QStringList &value)
{
    if (m_stringlist == value)
        return;

    beginResetModel();
    m_stringlist = value;
    endResetModel();
}

} // namespace SailfishConnect
