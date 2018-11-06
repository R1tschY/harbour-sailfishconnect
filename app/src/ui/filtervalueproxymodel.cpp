#include "filtervalueproxymodel.h"

#include <QModelIndex>
#include <QLoggingCategory>

namespace SailfishConnect {

FilterValueProxyModel::FilterValueProxyModel()
{
    connect(
        this, &QAbstractProxyModel::sourceModelChanged,
        this, &FilterValueProxyModel::updateFilterRole);
    connect(
        this, &QAbstractItemModel::modelReset,
        this, &FilterValueProxyModel::updateFilterRole);
}

bool FilterValueProxyModel::filterAcceptsRow(
        int source_row, const QModelIndex& source_parent) const
{
    QModelIndex sourceIndex1 = sourceModel()->index(
                source_row, filterKeyColumn(), source_parent);
    if (!m_complete || !m_filterValue.isValid())
        return false;

    Q_ASSERT(filterKeyColumn() >= 0); // TODO: -1 not supported
    QModelIndex sourceIndex = sourceModel()->index(
                source_row, filterKeyColumn(), source_parent);
    return m_filterValue == sourceModel()->data(sourceIndex, filterRole());
}

QString FilterValueProxyModel::filterRoleName() const
{
    return m_filterRoleName;
}

void FilterValueProxyModel::setFilterRoleName(const QString& filterRoleName)
{
    m_filterRoleName = filterRoleName;
    updateFilterRole();
}

QVariant FilterValueProxyModel::filterValue() const
{
    return m_filterValue;
}

void FilterValueProxyModel::setFilterValue(const QVariant &filterValue)
{
    m_filterValue = filterValue;
    if (m_complete)
        invalidateFilter();
}

void FilterValueProxyModel::classBegin()
{
    m_complete = false;
}

void FilterValueProxyModel::updateFilterRole()
{
    if (!m_complete)
        return;

    int role = roleNames().key(m_filterRoleName.toLatin1());
    setFilterRole(role);

    invalidateFilter();
}

void FilterValueProxyModel::componentComplete()
{
    m_complete = true;

    updateFilterRole();
}

} // namespace SailfishConnect
