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
