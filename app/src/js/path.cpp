#include "path.h"

#include <QQmlEngine>
#include <QtQml>

namespace QmlJs {

PathStatic::PathStatic(QObject *parent) : QObject(parent)
{

}

void Path::registerType()
{
    qmlRegisterSingletonType<PathStatic>(
                "SailfishConnect.Qml", 0, 4, "Path",
                [](QQmlEngine*, QJSEngine*) -> QObject* {
        return new PathStatic();
    });
    qmlRegisterType<Path>();
}

QString PathStatic::dirname(const QString &path)
{
    return QFileInfo(path).dir().path();
}

} // namespace QmlJs
