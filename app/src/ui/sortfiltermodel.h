#ifndef FILTERVALUEPROXYMODEL_H
#define FILTERVALUEPROXYMODEL_H

#include <QString>
#include <QVariant>
#include <QSortFilterProxyModel>
#include <QQmlParserStatus>

namespace SailfishConnect {

class SortFilterModel : public QSortFilterProxyModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(QString filterMode
               READ filterMode
               WRITE setFilterMode)

    Q_PROPERTY(QVariant filterValue
               READ filterValue
               WRITE setFilterValue)

    Q_PROPERTY(QString filterRole
               READ filterRole
               WRITE setFilterRole)

    Q_PROPERTY(QString sortRole
               READ sortRole
               WRITE setSortRole)

public:
    SortFilterModel();

    QVariant filterValue() const;
    void setFilterValue(const QVariant &filterValue);

    QString filterRole() const;
    void setFilterRole(const QString& filterRole);

    QString filterMode() const;
    void setFilterMode(const QString &filterMode);

    QString sortRole() const;
    void setSortRole(const QString& sortRole);

protected:
    bool filterAcceptsRow(
            int source_row, const QModelIndex& source_parent) const override;

private:
    int m_filterModeIndex = 1;
    QVariant m_filterValue;
    QString m_filterRole;
    QString m_sortRole;
    bool m_complete = true;

    void componentComplete() override;
    void classBegin() override;

    void invalidateFilter();
    void onRoleNamesChanged();
};

} // namespace SailfishConnect

#endif // FILTERVALUEPROXYMODEL_H
