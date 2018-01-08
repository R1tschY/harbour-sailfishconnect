/*
 * Copyright 2018 Richard Liebscher <richard.liebscher@gmail.com>.
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

#ifndef SSLHELPER_H
#define SSLHELPER_H

#include <openssl/bn.h>
#include <openssl/evp.h>
#include <openssl/x509.h>

#include <memory>
#include <QString>
#include <QDateTime>
#include <QMap>

class QDateTime;
class QSslCertificate;
class QSslKey;

namespace SailfishConnect {
namespace Ssl {

struct KeyGenerator {
    KeyGenerator() = delete;

    static QSslKey generateRsa(int bits);
};


enum CertificateInfoType {
    CommonName, Organization, Country, Locality, State, OrganizationalUnit
};

using CertificateInfo = QMap<CertificateInfoType, QString>;


class CertificateBuilder {
public:
    CertificateInfo getInfo() const { return info; }
    void setInfo(const CertificateInfo &value) { info = value; }

    int getSerialNumber() const { return serialNumber; }
    void setSerialNumber(int value) { serialNumber = value; }

    QDateTime getNotBefore() const { return notBefore; }
    void setNotBefore(const QDateTime &value) { notBefore = value; }

    QDateTime getNotAfter() const { return notAfter; }
    void setNotAfter(const QDateTime &value) { notAfter = value; }

    QSslCertificate selfSigned(const QSslKey& key) const;

private:
    CertificateInfo info;
    int serialNumber;
    QDateTime notBefore;
    QDateTime notAfter;
};

} // namespace Ssl
} // namespace SailfishConnect

#endif // SSLHELPER_H
