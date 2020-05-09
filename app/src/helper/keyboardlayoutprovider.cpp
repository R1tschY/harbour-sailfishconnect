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
#include <QDir>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QFeedbackEffect>
#include <QRegularExpression>
#include <QLoggingCategory>
#include <QStringBuilder>

static Q_LOGGING_CATEGORY(logger, "SailfishConnect.KeyboardLayoutProvider")

static QString MALIIT_LAYOUT_DIR = QStringLiteral("/usr/share/maliit/plugins/com/jolla/layouts/");

KeyboardLayoutProvider::KeyboardLayoutProvider(QObject *parent)
    : QObject(parent)
{
    m_settings.beginGroup(QStringLiteral("RemoteKeyboard"));
    m_layout = m_settings.value(QStringLiteral("layout"), QStringLiteral("en")).toString();
    m_repeatInterval = m_settings.value(QStringLiteral("repeatInterval"), 200).toInt();
    m_feedback = m_settings.value(QStringLiteral("keyboardFeedback"), true).toBool();
    m_settings.endGroup();

    QJsonArray row1;
    for (const QString& key : {"esc", "F1", "F2", "F3", "F4", "F5", "F6"}) {
        QJsonObject keyObject;
        keyObject.insert(QStringLiteral("caption"), key);
        if (key.startsWith('F')) {
            int num = key.at(1).toLatin1() - '0';
            QString name = 'F' % QString::number(num + 6);
            keyObject.insert(QStringLiteral("captionShifted"), name);
        }
        row1.append(keyObject);
    }
    m_row1 = row1.toVariantList();

    setLayout(m_layout);
    loadNames();
}

QString KeyboardLayoutProvider::layout() const
{
    return m_layout;
}

void KeyboardLayoutProvider::setLayout(const QString &layout)
{
    // seems to be the only way to get the files stored
    QFile layoutFile(MALIIT_LAYOUT_DIR % layout % ".qml");
    if (!layoutFile.open(QIODevice::ReadOnly)) {
        qCDebug(logger) << "Unknown layout: " << layout;
        return;
    }

    QRegularExpression caption("caption: \"([\\S])\"");
    QRegularExpression captionShifted("captionShifted: \"([\\S])\"");
    QRegularExpression symView("symView: \"([\\S]+)\"");
    QRegularExpression symView2("symView2: \"([\\S]+)\"");
    QByteArray line = layoutFile.readLine();
    QJsonArray keys;
    QJsonArray row;

    while (line.length() > 0) {
        if (line.contains("  }")) {
            keys.append(row);
        } else if (line.contains("KeyboardRow {")) {
            row = QJsonArray();
        } else if (line.contains("Key")) {
            QByteArray keySequence = line;
            while (!line.contains('}')) {
                line = layoutFile.readLine();
                keySequence.append(line);
            }
            while (keySequence.contains('\n')) {
                keySequence.replace('\n', ' ');
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
                QRegularExpressionMatch captionMatch =
                        caption.match(keySequence);
                QRegularExpressionMatch captionShiftedMatch =
                        captionShifted.match(keySequence);
                QRegularExpressionMatch symViewMatch =
                        symView.match(keySequence);
                QRegularExpressionMatch symView2Match =
                        symView2.match(keySequence);
                QJsonObject key;
                key["caption"] = captionMatch.captured(1);
                if (captionShiftedMatch.hasMatch()) {
                    key["captionShifted"] = captionShiftedMatch.captured(1);
                }
                if (symViewMatch.hasMatch()) {
                    key["symView"] = symViewMatch.captured(1);
                }
                if (symView2Match.hasMatch()) {
                    key["symView2"] = symView2Match.captured(1);
                }
                row.append(key);
            }
        }

        line = layoutFile.readLine();
    }

    layoutFile.close();

    m_settings.beginGroup(QStringLiteral("RemoteKeyboard"));
    m_settings.setValue(QStringLiteral("layout"), layout);
    m_settings.endGroup();

    m_row2 = keys[0].toArray().toVariantList();
    m_row3 = keys[1].toArray().toVariantList();
    m_row4 = keys[2].toArray().toVariantList();

    QJsonArray row5;
    for (const QString& key : {"?123", "ctrl", ",", " ", ".", "alt", "enter"}) {
        QJsonObject keyObject;
        keyObject["caption"] = key;
        if (key == ".") {
            keyObject["symView"] = "left";
        } else if (key == "alt") {
            keyObject["symView"] = "down";
        } else if (key == "enter") {
            keyObject["symView"] = "right";
        } else if (key == "?123") {
            keyObject["symView"] = "ABC";
        }
        row5.append(keyObject);
    }

    m_row5 = row5.toVariantList();

    m_layout = layout;

    emit layoutChanged();
}

QVariantList KeyboardLayoutProvider::layouts()
{
    return m_layouts;
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

int KeyboardLayoutProvider::repeatInterval() const
{
    return m_repeatInterval;
}

void KeyboardLayoutProvider::setRepeatInterval(const int &interval)
{
    m_repeatInterval = interval;

    m_settings.beginGroup(QStringLiteral("RemoteKeyboard"));
    m_settings.setValue(QStringLiteral("repeatInterval"), interval);
    m_settings.endGroup();

    emit settingsChanged();
}

bool KeyboardLayoutProvider::feedback() const
{
    return m_feedback;
}

void KeyboardLayoutProvider::setFeedback(const bool &feedback)
{
    m_feedback = feedback;

    m_settings.beginGroup(QStringLiteral("RemoteKeyboard"));
    m_settings.setValue(QStringLiteral("keyboardFeedback"), feedback);
    m_settings.endGroup();

    emit settingsChanged();
}

void KeyboardLayoutProvider::pressFeedback()
{
    QFeedbackHapticsEffect::playThemeEffect(QFeedbackEffect::PressStrong);
}

void KeyboardLayoutProvider::releaseFeedback()
{
    QFeedbackHapticsEffect::playThemeEffect(QFeedbackEffect::ReleaseStrong);
}

void KeyboardLayoutProvider::loadNames()
{
    QDir confDir(MALIIT_LAYOUT_DIR);
    QHash<QString, QString> longNames;

    // load configs first to get names
    for (const QString &conf : confDir.entryList()) {
        if (conf.endsWith(QStringLiteral(".conf"))) {
            QSettings settings(MALIIT_LAYOUT_DIR % conf, QSettings::NativeFormat);
            settings.setIniCodec("UTF-8");

            for (const QString &group : settings.childGroups()) {
                 settings.beginGroup(group);
                 longNames[group] = settings.value(
                     QStringLiteral("name")).toByteArray();
                 settings.endGroup();
            }
        }
    }

    // then load layouts
    for (const QString &layout : confDir.entryList()) {
        if (layout.endsWith(QStringLiteral(".qml")) && !layout.contains('_')) {
            // hi, kn, mr and te are currentently not working
            if (layout.contains(QStringLiteral("hi")) || layout.contains(QStringLiteral("kn")) ||
                    layout.contains(QStringLiteral("mr")) || layout.contains(QStringLiteral("te")) ||
                    layout.contains(QStringLiteral("emoji"))) {
                continue;
            }

            QJsonObject language;
            language.insert("short", layout.left(layout.indexOf('.')));
            language.insert("long", longNames[layout]);
            m_layouts.append(language);
        }
    }
}
