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

#ifndef STRINGLISTMODEL_H
#define STRINGLISTMODEL_H

#include <QStringListModel>

namespace SailfishConnect {

class StringListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QStringList stringList READ stringList WRITE setStringList NOTIFY stringListChanged)

public:
    enum ExtraRoles {
        DataRole = Qt::UserRole,
    };

    explicit StringListModel(QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    Q_SCRIPTABLE void append(const QString& value);
    Q_SCRIPTABLE bool removeAt(int index);
    Q_SCRIPTABLE bool removeOne(const QString& value);

    QStringList stringList() const { return m_stringlist; }
    void setStringList(const QStringList &value);

Q_SIGNALS:
    void stringListChanged();

private:
    QStringList m_stringlist;
};

} // namespace SailfishConnect

#endif // STRINGLISTMODEL_H
