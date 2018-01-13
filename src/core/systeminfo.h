#ifndef DEVICEINFO_H
#define DEVICEINFO_H

class QString;

namespace SailfishConnect {

class SystemInfo
{
public:
    virtual ~SystemInfo() = default;

    virtual QString defaultName() const;
    virtual QString deviceType() const;
};

} // namespace SailfishConnect

#endif // DEVICEINFO_H
