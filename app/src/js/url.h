/*
 * Copyright 2019 Richard Liebscher <richard.liebscher@gmail.com>.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef URL_H
#define URL_H

#include <QObject>
#include <QUrl>
#include <QVariant>

namespace QmlJs {

class Url
{
    Q_GADGET

    Q_PROPERTY(QString autority READ autority WRITE setAuthority)
    Q_PROPERTY(QString fragment READ fragment WRITE setFragment)
    Q_PROPERTY(QString host READ host WRITE setHost)
    Q_PROPERTY(QString password READ password WRITE setPassword)
    Q_PROPERTY(QString path READ path WRITE setPath)
    Q_PROPERTY(int port READ port WRITE setPort)
    Q_PROPERTY(QString query READ query WRITE setQuery)
    Q_PROPERTY(QString scheme READ scheme WRITE setScheme)
    Q_PROPERTY(QString userInfo READ userInfo WRITE setUserInfo)
    Q_PROPERTY(QString userName READ userName WRITE setUserName)

    Q_PROPERTY(QString fileName READ fileName)
public:
    static void registerType();

    explicit Url();
    explicit Url(const QUrl& url);
    explicit Url(const QString& url);

    QString autority() const { return url.authority(); }
    QString fileName() const { return url.fileName(); }
    QString fragment() const { return url.fragment(); }
    QString host() const { return url.host(); }
    QString password() const { return url.password(); }
    QString path() const { return url.path(); }
    int port() const { return url.port(); }
    QString query() const { return url.query(); }
    QString scheme() const { return url.scheme(); }
    QString userInfo() const { return url.userInfo(); }
    QString userName() const { return url.userName(); }

    void setAuthority(const QString &authority) { url.setAuthority(authority); }
    void setFragment(const QString &fragment) { url.setFragment(fragment); }
    void setHost(const QString &host) { url.setHost(host); }
    void setPassword(const QString &password) { url.setPassword(password); }
    void setPath(const QString &path) { url.setPath(path); }
    void setPort(int port) { url.setPort(port); }
    void setQuery(const QString &query) { url.setQuery(query); }
    void setScheme(const QString &scheme) { url.setScheme(scheme); }
    void setUserInfo(const QString &userInfo) { url.setUserInfo(userInfo); }
    void setUserName(const QString &userName) { url.setUserName(userName); }

    Q_INVOKABLE void clear() { url.clear(); }
    Q_INVOKABLE QString toString() { return url.toString(); }

private:
    QUrl url;
};

class UrlStatic : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE QVariant fromUrl(const QUrl& url) {
        return QVariant::fromValue(Url(url));
    }

    Q_INVOKABLE QVariant fromString(const QString& str) {
        return QVariant::fromValue(Url(str));
    }
};

} // namespace QmlJs

Q_DECLARE_METATYPE(QmlJs::Url)

#endif // URL_H
