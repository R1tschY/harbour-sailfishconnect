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
#include <QSettings>

class KeyboardLayoutProvider : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString layout READ layout WRITE setLayout NOTIFY layoutChanged)
    Q_PROPERTY(QVariantList row1 READ row1 NOTIFY layoutChanged)
    Q_PROPERTY(QVariantList row2 READ row2 NOTIFY layoutChanged)
    Q_PROPERTY(QVariantList row3 READ row3 NOTIFY layoutChanged)
    Q_PROPERTY(QVariantList row4 READ row4 NOTIFY layoutChanged)
    Q_PROPERTY(int repeatInterval READ repeatInterval WRITE setRepeatInterval NOTIFY settingsChanged)
    Q_PROPERTY(bool feedback READ feedback WRITE setFeedback NOTIFY settingsChanged)
public:
    explicit KeyboardLayoutProvider(QObject *parent = nullptr);

    QString layout() const;
    void setLayout(const QString &layout);

    Q_INVOKABLE QVariantList layouts();

    QVariantList row1() const;
    QVariantList row2() const;
    QVariantList row3() const;
    QVariantList row4() const;

    int repeatInterval() const;
    void setRepeatInterval(const int &interval);

    bool feedback() const;
    void setFeedback(const bool &feedback);

    Q_INVOKABLE void pressFeedback();
    Q_INVOKABLE void releaseFeedback();

signals:
    void layoutChanged();
    void settingsChanged();

private:
    QString m_layout;
    QVariantList m_row1;
    QVariantList m_row2;
    QVariantList m_row3;
    QVariantList m_row4;
    int m_repeatInterval;
    bool m_feedback;
    QSettings m_settings;
};

#endif // KEYBOARDLAYOUTPROVIDER_H
