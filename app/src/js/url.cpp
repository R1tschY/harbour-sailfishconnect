#include "url.h"

#include <QQmlEngine>
#include <QtQml>

namespace QmlJs {

void Url::registerType()
{
    qmlRegisterSingletonType<UrlStatic>(
                "SailfishConnect.Qml", 0, 3, "Url",
                [](QQmlEngine*, QJSEngine*) -> QObject* {
        return new UrlStatic();
    });
    qmlRegisterType<Url>();
}

Url::Url() = default;

Url::Url(const QUrl& url)
    : url(std::move(url))
{ }

Url::Url(const QString &url)
    : url(url)
{ }


} // namespace QmlJs
