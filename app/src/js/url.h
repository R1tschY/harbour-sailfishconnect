#ifndef URL_H
#define URL_H

#include <QObject>
#include <QUrl>

namespace QmlJs {

class Url : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString scheme READ scheme WRITE setScheme)
public:
    static void registerType();

    explicit Url(const QUrl& url);

    Q_INVOKABLE static Url* fromUrl(const QUrl& url) { return new Url(url); }
//    Q_INVOKABLE static Url* fromUser();

    QString scheme() const { return url.scheme(); }
    void setScheme(const QString& scheme) { url.setScheme(scheme); }

    QString path() const { return url.path(); }
    void setPath(const QString& path) { url.setPath(path); }

    QString fileName() const { return url.fileName(); }

    Q_INVOKABLE void clear() { url.clear(); }

private:
    QUrl url;
};

} // namespace QmlJs

#endif // URL_H
