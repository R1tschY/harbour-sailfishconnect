#pragma once

#include <QObject>

namespace QmlJs {

class Humanize : public QObject
{
    Q_OBJECT
public:
    explicit Humanize(QObject *parent = nullptr);

    Q_INVOKABLE static QString bytes(qint64 bytes);

    static void registerType();
};

} // namespace QmlJs
