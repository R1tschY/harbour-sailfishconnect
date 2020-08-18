/*
 * Copyright 2020 Richard Liebscher <richard.liebscher@gmail.com>.
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

#include "hostaddressvalidator.h"

#include <QQmlEngine>
#include <QtQml>

namespace QmlJs {

HostAddressValidator::HostAddressValidator(QObject* parent)
    : QValidator(parent)
{
}

void HostAddressValidator::registerType()
{
    qmlRegisterType<HostAddressValidator>("SailfishConnect.Qml", 0, 6, "HostAddressValidator");
}

QValidator::State HostAddressValidator::validate(QString& input, int& pos) const
{
    QHostAddress address(input);
    return address.isNull() ? QValidator::Intermediate : QValidator::Acceptable;
}

} // namespace QmlJs
