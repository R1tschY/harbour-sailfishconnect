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

#include "humanize.h"

#include <array>
#include <cmath>
#include <algorithm>

namespace SailfishConnect {

static std::array<QString, 5> bytesUnits{
    QStringLiteral(" B"),
    QStringLiteral(" kB"),
    QStringLiteral(" MB"),
    QStringLiteral(" GB"),
    QStringLiteral(" TB")
};

QString humanizeBytes(qint64 bytes) {
    std::size_t dim;
    if (bytes != 0) {
        dim = std::log(std::abs(bytes)) / std::log(1024);
        dim = std::min(dim, bytesUnits.size() - 1);
    } else {
        dim = 0;
    }

    if (dim != 0) {
        return QString::number(bytes / pow(1024, dim), 'f', 2) + bytesUnits[dim];
    } else {
        return QString::number(bytes) + bytesUnits[0];
    }
}

} // namespace SailfishConnect
