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
#include <QRegularExpression>

KeyboardLayoutProvider::KeyboardLayoutProvider(QObject *parent) : QObject(parent)
{
    m_layout = "de";
    m_repeatInterval = m_settings.value("repeatInterval", 200).toInt();
    m_feedback = m_settings.value("keyboardFeedback", true).toBool();
    setLayout(m_layout);
}

QString KeyboardLayoutProvider::layout() const
{
    return m_layout;
}

void KeyboardLayoutProvider::setLayout(const QString &layout)
{
    // seems to be the only way to get the files stored
    QFile layoutFile("/usr/share/maliit/plugins/com/jolla/layouts/" + layout + ".qml");
    if (!layoutFile.open(QIODevice::ReadOnly)) {
        qDebug() << "unkown layout: " << layout;
        return;
    }

    QRegularExpression caption("caption: \"([\\S])\"");
    QRegularExpression captionShifted("captionShifted: \"([\\S])\"");
    QRegularExpression symView("symView: \"([\\S]+)\"");
    QRegularExpression symView2("symView2: \"([\\S]+)\"");
    QByteArray line;
    line = layoutFile.readLine();
    QByteArray keySequence;
    QJsonArray keys;
    QJsonArray row;

    while (line.length() > 0) {
        if (line.contains("  }")) {
            keys.append(row);
        } else if (line.contains("KeyboardRow {")) {
            row = QJsonArray();
        } else if (line.contains("Key")) {
            keySequence.clear();
            keySequence.append(line);
            while (!line.contains("}")) {
                line = layoutFile.readLine();
                keySequence.append(line);
            }
            while (keySequence.contains("\n")) {
                keySequence.replace("\n", " ");
            }

            // remove backslash
            keySequence.replace("\\\"", "\"");
            keySequence.replace("\\\\", "\\");

            if (keySequence.contains("ShiftKey")) {
                QJsonObject key;
                key["caption"] = "shift";
                row.append(key);
            } else if (keySequence.contains("BackspaceKey")) {
                QJsonObject upKey;
                upKey["caption"] = "";
                upKey["symView"] = "up";
                row.append(upKey);
                QJsonObject key;
                key["caption"] = "backspace";
                row.append(key);
            } else {
                QRegularExpressionMatch captionMatch = caption.match(keySequence);
                QRegularExpressionMatch captionShiftedMatch = captionShifted.match(keySequence);
                QRegularExpressionMatch symViewMatch = symView.match(keySequence);
                QRegularExpressionMatch symView2Match = symView2.match(keySequence);
                QJsonObject key;
                key["caption"] = captionMatch.captured(1);
                if (captionShiftedMatch.hasMatch()) key["captionShifted"] = captionShiftedMatch.captured(1);
                if (symViewMatch.hasMatch()) key["symView"] = symViewMatch.captured(1);
                if (symView2Match.hasMatch()) key["symView2"] = symView2Match.captured(1);
                row.append(key);
            }
        }

        line = layoutFile.readLine();
    }

    layoutFile.close();

    m_row1 = keys[0].toArray().toVariantList();
    m_row2 = keys[1].toArray().toVariantList();
    m_row3 = keys[2].toArray().toVariantList();

    QJsonArray row4;
    for (QString key : {"char", "ctrl", ",", " ", ".", "alt", "enter"}) {
        QJsonObject keyObject;
        keyObject["caption"] = key;
        if (key == ".") {
            keyObject["symView"] = "left";
        } else if (key == "alt") {
            keyObject["symView"] = "down";
        } else if (key == "enter") {
            keyObject["symView"] = "right";
        }
        row4.append(keyObject);
    }

    m_row4 = row4.toVariantList();

    m_layout = layout;

    emit layoutChanged();
}

QVariantList KeyboardLayoutProvider::layouts()
{
    QDir layoutDir("/usr/share/maliit/plugins/com/jolla/layouts/");

    layoutDir.setSorting(QDir::Name);

    QVariantList layouts;
    for (QString layout : layoutDir.entryList()) {
        // layouts with _ are not working
        if (!layout.startsWith(".") && !layout.contains("_")) {
            // hi, kn, mr and te are currentently not working
            if (layout.contains("hi") || layout.contains("kn") || layout.contains("mr") || layout.contains("te")) continue;
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

int KeyboardLayoutProvider::repeatInterval() const
{
    return m_repeatInterval;
}

void KeyboardLayoutProvider::setRepeatInterval(const int &interval)
{
    m_repeatInterval = interval;

    m_settings.setValue("repeatInterval", interval);

    emit settingsChanged();
}

bool KeyboardLayoutProvider::feedback() const
{
    return m_feedback;
}

void KeyboardLayoutProvider::setFeedback(const bool &feedback)
{
    m_feedback = feedback;

    m_settings.setValue("keyboardFeedback", feedback);

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
