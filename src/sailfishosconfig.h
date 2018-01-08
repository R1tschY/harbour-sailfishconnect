#ifndef SAILFISHOSCONFIG_H
#define SAILFISHOSCONFIG_H

#include "core/kdeconnectconfig.h"

namespace SailfishConnect {

class SailfishOsConfig : public KdeConnectConfig
{
public:
    QString defaultName() const override;
    QString deviceType() const override;
};

} // namespace SailfishConnect

#endif // SAILFISHOSCONFIG_H
