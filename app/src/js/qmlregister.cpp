#include "qmlregister.h"

#include "url.h"

namespace QmlJs {

void registerTypes()
{
    Url::registerType();
}

} // namespace QmlJs
