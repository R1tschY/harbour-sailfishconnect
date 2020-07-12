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

#include "filehelper.h"

#include <QDir>
#include <QDebug>
#include <QRegularExpression>

namespace SailfishConnect {

// TODO: Use KFileUtils::suggestName
QFileInfo nonexistingFile(const QFileInfo& path) {
    QFileInfo result = path;
    const QString prefix = result.baseName();
    QString suffix = result.completeSuffix();
    if (path.fileName().contains(QChar('.'))) {
        suffix.insert(0, QChar('.'));
    }

    if (!prefix.isEmpty()) {
        int i = 0;
        while (result.exists()) {
            i += 1;
            result.setFile(
                result.dir(),
                QStringLiteral("%1 (%2)%3").arg(
                    prefix, QString::number(i), suffix));
        }
        return result;
    } else {
        int i = 0;
        while (result.exists()) {
            i += 1;
            result.setFile(
                result.dir(),
                QStringLiteral("%1 (%2)").arg(suffix, QString::number(i)));
        }
        return result;
    }
}

QString escapeForFilePath(const QString& name)
{
    QString result = name.trimmed();
    result.replace(QChar('/'), QString());
    result.replace(QChar('\0'), QString());

    if (result.isEmpty()
            || result == QLatin1String(".")
            || result == QLatin1String("..")) {
        return QStringLiteral("_");
    } else {
        return result;
    }
}

} // namespace SailfishConnect
