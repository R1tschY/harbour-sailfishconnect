#ifndef FILTERVALUEPROXYMODEL_H
#define FILTERVALUEPROXYMODEL_H

#include <QString>
#include <QVariant>
#include <QSortFilterProxyModel>
#include <QQmlParserStatus>

namespace SailfishConnect {

class FilterValueProxyModel : public QSortFilterProxyModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(QVariant filterValue
               READ filterValue
               WRITE setFilterValue
               NOTIFY filterValueChanged)

    Q_PROPERTY(QString filterRoleName
               READ filterRoleName
               WRITE setFilterRoleName)

public:
    FilterValueProxyModel();

    QVariant filterValue() const;
    void setFilterValue(const QVariant &filterValue);

    QString filterRoleName() const;
    void setFilterRoleName(const QString& filterRoleName);

signals:
    void filterValueChanged();

protected:
    bool filterAcceptsRow(
            int source_row, const QModelIndex& source_parent) const override;

private:
    QVariant m_filterValue;
    QString m_filterRoleName;
    bool m_complete = true;

    void componentComplete() override;
    void classBegin() override;

    void updateFilterRole();
};

} // namespace SailfishConnect

#endif // FILTERVALUEPROXYMODEL_H
