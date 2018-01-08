#include "sailfishosconfig.h"

#include <QMap>
#include <QString>
#include <QSettings>

namespace SailfishConnect {

QString SailfishOsConfig::defaultName() const
{
    QSettings hwRelease(
                QStringLiteral("/etc/hw-release"), QSettings::IniFormat);
    auto hwName = hwRelease.value(QStringLiteral("NAME")).toString();
    if (!hwName.isEmpty()) {
        return hwName;
    }

    return KdeConnectConfig::defaultName();
}

QString SailfishOsConfig::deviceType() const
{
    // TODO: How to detect tablet?
    return QStringLiteral("phone");
}

} // namespace SailfishConnect
