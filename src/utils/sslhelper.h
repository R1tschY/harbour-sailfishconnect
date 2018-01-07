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
