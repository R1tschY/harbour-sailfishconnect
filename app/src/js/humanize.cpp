#include "humanize.h"

#include <QQmlEngine>
#include <QtQml>

#include <sailfishconnect/helper/humanize.h>

using namespace SailfishConnect;

namespace QmlJs {

Humanize::Humanize(QObject *parent) : QObject(parent)
{

}

QString Humanize::bytes(qint64 bytes)
{
    return humanizeBytes(bytes);
}

void Humanize::registerType()
{
    qmlRegisterSingletonType<Humanize>(
                "SailfishConnect.Qml", 0, 3, "Humanize",
                [](QQmlEngine*, QJSEngine*) -> QObject* {
        return new Humanize();
    });
}

} // namespace QmlJs
