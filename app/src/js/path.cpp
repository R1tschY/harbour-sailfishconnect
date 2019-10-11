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

#include "path.h"

#include <QQmlEngine>
#include <QtQml>

namespace QmlJs {

PathStatic::PathStatic(QObject *parent) : QObject(parent)
{

}

void Path::registerType()
{
    qmlRegisterSingletonType<PathStatic>(
                "SailfishConnect.Qml", 0, 4, "Path",
                [](QQmlEngine*, QJSEngine*) -> QObject* {
        return new PathStatic();
    });
    qmlRegisterType<Path>();
}

QString PathStatic::dirname(const QString &path)
{
    return QFileInfo(path).dir().path();
}

} // namespace QmlJs
