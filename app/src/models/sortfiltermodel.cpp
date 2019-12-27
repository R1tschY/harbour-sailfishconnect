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

#include "sortfiltermodel.h"

#include <QModelIndex>
#include <QLoggingCategory>

namespace SailfishConnect {

static const QList<QString> filterModes {
    QStringLiteral("none"),
    QStringLiteral("value"),
    QStringLiteral("regex"),
};

SortFilterModel::SortFilterModel()
{
    connect(
        this, &QAbstractProxyModel::sourceModelChanged,
        this, &SortFilterModel::onRoleNamesChanged);
    connect(
        this, &QAbstractItemModel::modelReset,
        this, &SortFilterModel::onRoleNamesChanged);
}

bool SortFilterModel::filterAcceptsRow(
        int source_row, const QModelIndex& source_parent) const
{
    if (m_filterModeIndex == 0) {
        return true;
    }

    if (m_filterModeIndex > 1) {
        return QSortFilterProxyModel::filterAcceptsRow(
                    source_row, source_parent);
    }

    if (!m_complete || !m_filterValue.isValid())
        return false;

    Q_ASSERT(filterKeyColumn() >= 0); // TODO: -1 not supported
    QModelIndex sourceIndex = sourceModel()->index(
                source_row, filterKeyColumn(), source_parent);
    return m_filterValue == sourceModel()->data(
                sourceIndex, QSortFilterProxyModel::filterRole());
}

bool SortFilterModel::sortAscending() const
{
    return m_sortAscending;
}

void SortFilterModel::setSortAscending(bool sortAscending)
{
    if (m_sortAscending == sortAscending)
        return;

    m_sortAscending = sortAscending;

    if (m_complete && !m_sortRole.isEmpty())
        sort(0, m_sortAscending ? Qt::AscendingOrder : Qt::DescendingOrder);
}

QString SortFilterModel::filterMode() const
{
    return filterModes[m_filterModeIndex];
}

void SortFilterModel::setFilterMode(const QString &filterMode)
{   
    int index = filterModes.indexOf(filterMode);
    if (index < 0)
        return;

    if (index == m_filterModeIndex)
        return;

    m_filterModeIndex = index;

    invalidateFilter();
}

QString SortFilterModel::sortRole() const
{
    return m_sortRole;
}

void SortFilterModel::setSortRole(const QString &sortRoleName)
{
    if (sortRoleName == m_sortRole)
        return;

    m_sortRole = sortRoleName;

    if (m_complete)
        QSortFilterProxyModel::setSortRole(
                    roleNames().key(m_sortRole.toLatin1(), 0));
}

void SortFilterModel::setFilterRole(const QString& filterRoleName)
{
    if (filterRoleName == m_filterRole)
        return;

    m_filterRole = filterRoleName;

    if (m_complete)
        QSortFilterProxyModel::setFilterRole(
                    roleNames().key(m_filterRole.toLatin1(), 0));
}

QVariant SortFilterModel::filterValue() const
{
    return m_filterValue;
}

void SortFilterModel::setFilterValue(const QVariant &filterValue)
{
    if (filterValue == m_filterValue)
        return;

    m_filterValue = filterValue;

    invalidateFilter();
}

QString SortFilterModel::filterRole() const
{
    return m_filterRole;
}

void SortFilterModel::classBegin()
{
    m_complete = false;
}

void SortFilterModel::invalidateFilter()
{
    if (!m_complete)
        return;

    if (m_filterModeIndex == 2) {
        // regex
        setFilterRegExp(m_filterValue.toString());
    }

    QSortFilterProxyModel::invalidateFilter();
}

void SortFilterModel::onRoleNamesChanged()
{
    if (!m_complete)
        return;

    const auto roles = roleNames();
    QSortFilterProxyModel::setSortRole(roles.key(m_sortRole.toLatin1(), 0));
    QSortFilterProxyModel::setFilterRole(roles.key(m_filterRole.toLatin1(), 0));
}

void SortFilterModel::componentComplete()
{
    m_complete = true;
    onRoleNamesChanged();
    invalidateFilter();

    if (!m_sortRole.isEmpty()) {
        sort(0, m_sortAscending ? Qt::AscendingOrder : Qt::DescendingOrder);
    }
}

} // namespace SailfishConnect
