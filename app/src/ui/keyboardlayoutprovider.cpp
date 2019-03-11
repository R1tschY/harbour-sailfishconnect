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
#include <QtFeedback/QFeedbackEffect>

KeyboardLayoutProvider::KeyboardLayoutProvider(QObject *parent) : QObject(parent)
{
    m_layout = "english";
    m_repeatInterval = 200;
    m_feedback = true;
    QFile saved(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/harbour-sailfishconnect/keyboardlayout.conf");
    if (saved.exists()) {
        saved.open(QIODevice::ReadOnly);
        QJsonObject conf = QJsonDocument::fromJson(saved.readAll()).object();
        saved.close();
        m_layout = conf["layout"].toString();
        m_repeatInterval = conf["interval"].toInt();
        m_feedback = conf["feedback"].toBool();
    }
    setLayout(m_layout);
}

QString KeyboardLayoutProvider::layout() const
{
    return m_layout;
}

void KeyboardLayoutProvider::setLayout(const QString &layout)
{
    // seems to be the only way to get the files stored
    QFile layoutFile(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/keyboard-layouts/" + layout + ".json");
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

    saveConfig();

    emit layoutChanged();
}

QVariantList KeyboardLayoutProvider::layouts()
{
    QDir layoutDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/keyboard-layouts/");

    layoutDir.setSorting(QDir::Name);

    QVariantList layouts;
    for (QString layout : layoutDir.entryList()) {
        if (!layout.startsWith(".")) {
            layouts.append(layout.left(layout.indexOf(".")));
        }
    }

    return layouts;
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

int KeyboardLayoutProvider::repeatInterval() const
{
    return m_repeatInterval;
}

void KeyboardLayoutProvider::setRepeatInterval(const int &interval)
{
    m_repeatInterval = interval;

    saveConfig();

    emit settingsChanged();
}

bool KeyboardLayoutProvider::feedback() const
{
    return m_feedback;
}

void KeyboardLayoutProvider::setFeedback(const bool &feedback)
{
    m_feedback = feedback;

    saveConfig();

    emit settingsChanged();
}

void KeyboardLayoutProvider::pressFeedback()
{
    QFeedbackHapticsEffect::playThemeEffect(QFeedbackEffect::PressWeak);
}

void KeyboardLayoutProvider::releaseFeedback()
{
    QFeedbackHapticsEffect::playThemeEffect(QFeedbackEffect::ReleaseWeak);
}

void KeyboardLayoutProvider::saveConfig() const
{
    QFile config(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/harbour-sailfishconnect/keyboardlayout.conf");
    config.open(QIODevice::WriteOnly);

    QJsonObject conf;
    conf["layout"] = m_layout;
    conf["interval"] = m_repeatInterval;
    conf["feedback"] = m_feedback;

    config.write(QJsonDocument(conf).toJson());
    config.close();
}
