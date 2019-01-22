#include "qmlregister.h"

#include "url.h"
#include "humanize.h"

namespace QmlJs {

void registerTypes()
{
    Url::registerType();
    Humanize::registerType();
}

} // namespace QmlJs
