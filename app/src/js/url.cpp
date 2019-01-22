#include "url.h"

namespace QmlJs {

void Url::registerType()
{

}

Url::Url(const QUrl& url)
{

}

Url *Url::fromUrl(const QUrl &url)
{
    return new Url(url);
}

} // namespace QmlJs
