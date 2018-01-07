#include "sslhelper.h"

#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/pem.h>

#include <QDebug>
#include <QDateTime>
#include <QLoggingCategory>
#include <QIODevice>
#include <QSslKey>
#include <QSslCertificate>

namespace SailfishConnect {
namespace Ssl {

static Q_LOGGING_CATEGORY(logger, "sailfishconnect.ssl")

struct X509Deleter {
    void operator()(X509* ptr) { X509_free(ptr); }
};
using X509Ptr = std::unique_ptr<X509, X509Deleter>;

struct BioDeleter {
    void operator()(BIO* ptr) { BIO_free(ptr); }
};
using BioPtr = std::unique_ptr<BIO, BioDeleter>;

struct EvpPkeyDeleter {
    void operator()(EVP_PKEY* ptr) { EVP_PKEY_free(ptr); }
};
using EvpPkeyPtr = std::unique_ptr<EVP_PKEY, EvpPkeyDeleter>;

struct BigNumDeleter {
    void operator()(BIGNUM* ptr) { BN_free(ptr); }
};
using BigNumPtr = std::unique_ptr<BIGNUM, BigNumDeleter>;



static QByteArray getByteArray(BIO* bio)
{
    QByteArray buf;

    int ret;
    do {
        char block[1024];
        ret = BIO_read(bio, block, 1024);
        if (ret <= 0)
            break;

        buf.append(block, ret);
    } while (ret == 1024);

    return buf;
}

static BigNumPtr toBigNum(unsigned int value)
{
    BigNumPtr result(BN_new());

    int success = BN_set_word(result.get(), value);
    if (!success) {
        BN_zero(result.get());
        qCWarning(logger) << "BN_set_word failed";
    }

    return result;
}

QSslKey KeyGenerator::generateRsa(int bits)
{
    EvpPkeyPtr result(EVP_PKEY_new());
    BigNumPtr e = toBigNum(RSA_F4);

    RSA* rsa = RSA_new();
    bool success = RSA_generate_key_ex(rsa, bits, e.get(), nullptr);
    if (!success) {
        return QSslKey();
    }

    success = EVP_PKEY_assign_RSA(result.get(), rsa);
    if (!success) {
        return QSslKey();
    }

    return QSslKey(
        reinterpret_cast<Qt::HANDLE>(result.release()),
        QSsl::PrivateKey);
}


static void addInfoEntry(X509_NAME* name, int nid, const QString &val)
{
    if (val.isNull())
        return;

    QByteArray buf = val.toLatin1();
    X509_NAME_add_entry_by_NID(
        name, nid,
        MBSTRING_ASC, (unsigned char *)buf.data(), buf.size(), -1, 0);
}

QSslCertificate CertificateBuilder::selfSigned(const QSslKey &key) const
{
    if (key.type() != QSsl::PrivateKey)
        return QSslCertificate();

    X509Ptr cert(X509_new());

    // serial number
    ASN1_INTEGER_set(X509_get_serialNumber(cert.get()), serialNumber);

    // notBefore / notAfter
    ASN1_TIME_set(
       X509_get_notBefore(cert.get()),
       notBefore.toMSecsSinceEpoch() / 1000);
    ASN1_TIME_set(
       X509_get_notAfter(cert.get()),
       notAfter.toMSecsSinceEpoch() / 1000);

    // key
    EvpPkeyPtr pkey(EVP_PKEY_new());
    switch (key.algorithm()) {
    case QSsl::Rsa:
        EVP_PKEY_set1_RSA(pkey.get(), reinterpret_cast<RSA*>(key.handle()));
        break;

    case QSsl::Dsa:
        EVP_PKEY_set1_DSA(pkey.get(), reinterpret_cast<DSA*>(key.handle()));
        break;

    case QSsl::Ec:
        EVP_PKEY_set1_EC_KEY(pkey.get(), reinterpret_cast<EC_KEY*>(key.handle()));
        break;

    default:
        return QSslCertificate();
    }
    X509_set_pubkey(cert.get(), pkey.get());

    // informations
    X509_NAME* name;
    name = X509_get_subject_name(cert.get());

    addInfoEntry(name, NID_commonName, info.value(CommonName));
    addInfoEntry(name, NID_countryName, info.value(Country));
    addInfoEntry(name, NID_localityName, info.value(Locality));
    addInfoEntry(name, NID_stateOrProvinceName, info.value(State));
    addInfoEntry(name, NID_organizationName, info.value(Organization));
    addInfoEntry(name, NID_organizationalUnitName,
                info.value(OrganizationalUnit));

    // subject == issuer
    X509_set_issuer_name(cert.get(), name);

    // sign
    int bytes = X509_sign(cert.get(), pkey.get(), EVP_sha1());
    if (bytes == 0) {
        qCWarning(logger) << "certificate signing failed (X509_sign)";
        return QSslCertificate();
    }

    // convert to PEM
    BioPtr mem(BIO_new(BIO_s_mem()));
    bool success = PEM_write_bio_X509(mem.get(), cert.get());
    if (!success) {
        qCWarning(logger) << "convertion of certificate to PEM format failed";
        return QSslCertificate();
    }

    // create QSslCertificate
    return QSslCertificate(getByteArray(mem.get()), QSsl::Pem);
}

} // namespace Ssl
} // SailfishConnect
