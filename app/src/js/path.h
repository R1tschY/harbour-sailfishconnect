#ifndef PATH_H
#define PATH_H

#include <QObject>

namespace QmlJs {

class PathStatic : public QObject
{
    Q_OBJECT
public:
    explicit PathStatic(QObject *parent = nullptr);

    void fromString(const QString& path);

    Q_SCRIPTABLE QString dirname(const QString& path);



signals:

public slots:
};

class Path : public QObject
{
    Q_OBJECT
public:

    static void registerType();
};

} // namespace QmlJs

#endif // PATH_H
