#include "systeminfo.h"

#include <QString>
#include <QHostInfo>

namespace SailfishConnect {

QString SystemInfo::defaultName() const
{
    return qgetenv("USER") + '@' + QHostInfo::localHostName();
}

QString SystemInfo::deviceType() const
{
    return QStringLiteral("unknown");
}

} // namespace SailfishConnect
