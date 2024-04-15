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
    for (const QString& key : {QStringLiteral("esc"), QStringLiteral("F1"), QStringLiteral("F2"), QStringLiteral("F3"), QStringLiteral("F4"), QStringLiteral("F5"), QStringLiteral("F6")}) {
        QJsonObject keyObject;
        keyObject.insert(QStringLiteral("caption"), key);
        if (key.startsWith(QChar::fromLatin1('F'))) {
            int num = key.at(1).toLatin1() - '0';
            QString name = QChar::fromLatin1('F') % QString::number(num + 6);
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
    QFile layoutFile(MALIIT_LAYOUT_DIR % layout % QStringLiteral(".qml"));
    if (!layoutFile.open(QIODevice::ReadOnly)) {
        qCDebug(logger) << "Unknown layout: " << layout;
        return;
    }

    QRegularExpression caption(QStringLiteral("caption: \"([\\S])\""));
    QRegularExpression captionShifted(QStringLiteral("captionShifted: \"([\\S])\""));
    QRegularExpression symView(QStringLiteral("symView: \"([\\S]+)\""));
    QRegularExpression symView2(QStringLiteral("symView2: \"([\\S]+)\""));
    QString line = QString::fromUtf8(layoutFile.readLine());
    QJsonArray keys;
    QJsonArray row;

    while (line.length() > 0) {
        if (line.contains(QStringLiteral("  }"))) {
            keys.append(row);
        } else if (line.contains(QStringLiteral("KeyboardRow {"))) {
            row = QJsonArray();
        } else if (line.contains(QStringLiteral("Key"))) {
            QString keySequence = line;
            while (!line.contains(QChar::fromLatin1('}'))) {
                line = QString::fromUtf8(layoutFile.readLine());
                keySequence.append(line);
            }
            while (keySequence.contains(QChar::fromLatin1('\n'))) {
                keySequence.replace(QChar::fromLatin1('\n'), QChar::fromLatin1(' '));
            }

            // remove backslash
            keySequence.replace(QStringLiteral("\\\""), QStringLiteral("\""));
            keySequence.replace(QStringLiteral("\\\\"), QStringLiteral("\\"));

            if (keySequence.contains(QStringLiteral("ShiftKey"))) {
                QJsonObject key;
                key[QStringLiteral("caption")] = QStringLiteral("shift");
                row.append(key);
            } else if (keySequence.contains(QStringLiteral("BackspaceKey"))) {
                QJsonObject upKey;
                upKey[QStringLiteral("caption")] = QStringLiteral("");
                upKey[QStringLiteral("symView")] = QStringLiteral("up");
                row.append(upKey);
                QJsonObject key;
                key[QStringLiteral("caption")] = QStringLiteral("backspace");
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
                key[QStringLiteral("caption")] = captionMatch.captured(1);
                if (captionShiftedMatch.hasMatch()) {
                    key[QStringLiteral("captionShifted")] = captionShiftedMatch.captured(1);
                }
                if (symViewMatch.hasMatch()) {
                    key[QStringLiteral("symView")] = symViewMatch.captured(1);
                }
                if (symView2Match.hasMatch()) {
                    key[QStringLiteral("symView2")] = symView2Match.captured(1);
                }
                row.append(key);
            }
        }

        line = QString::fromUtf8(layoutFile.readLine());
    }

    layoutFile.close();

    m_settings.beginGroup(QStringLiteral("RemoteKeyboard"));
    m_settings.setValue(QStringLiteral("layout"), layout);
    m_settings.endGroup();

    m_row2 = keys[0].toArray().toVariantList();
    m_row3 = keys[1].toArray().toVariantList();
    m_row4 = keys[2].toArray().toVariantList();

    QJsonArray row5;
    for (const QString& key : {QStringLiteral("?123"), QStringLiteral("ctrl"), QStringLiteral(","), QStringLiteral(" "), QStringLiteral("."), QStringLiteral("alt"), QStringLiteral("enter")}) {
        QJsonObject keyObject;
        keyObject[QStringLiteral("caption")] = key;
        if (key == QStringLiteral(".")) {
            keyObject[QStringLiteral("symView")] = QStringLiteral("left");
        } else if (key == QStringLiteral("alt")) {
            keyObject[QStringLiteral("symView")] = QStringLiteral("down");
        } else if (key == QStringLiteral("enter")) {
            keyObject[QStringLiteral("symView")] = QStringLiteral("right");
        } else if (key == QStringLiteral("?123")) {
            keyObject[QStringLiteral("symView")] = QStringLiteral("ABC");
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
#ifndef BUILD_FOR_HARBOUR
    QFeedbackHapticsEffect::playThemeEffect(QFeedbackEffect::PressStrong);
#endif
}

void KeyboardLayoutProvider::releaseFeedback()
{
#ifndef BUILD_FOR_HARBOUR
    QFeedbackHapticsEffect::playThemeEffect(QFeedbackEffect::ReleaseStrong);
#endif
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
                 longNames[group] = settings.value(QStringLiteral("name")).toString();
                 settings.endGroup();
            }
        }
    }

    // then load layouts
    for (const QString &layout : confDir.entryList()) {
        if (layout.endsWith(QStringLiteral(".qml")) && !layout.contains(QChar::fromLatin1('_'))) {
            // hi, kn, mr and te are currentently not working
            if (layout.contains(QStringLiteral("hi")) || layout.contains(QStringLiteral("kn")) ||
                    layout.contains(QStringLiteral("mr")) || layout.contains(QStringLiteral("te")) ||
                    layout.contains(QStringLiteral("emoji"))) {
                continue;
            }

            QJsonObject language;
            language.insert(QStringLiteral("short"), layout.left(layout.indexOf(QChar::fromLatin1('.'))));
            language.insert(QStringLiteral("long"), longNames[layout]);
            m_layouts.append(language);
        }
    }
}
