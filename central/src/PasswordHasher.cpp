//
// Created by robin on 11/22/2024.
//

#include "PaswordHasher.h"

#include <QDebug>
#include <QString>
#include <openssl/evp.h>
#include <openssl/rand.h>


QByteArray PasswordHasher::hashPassword(const QString& password, const QString& salt, int iterations)
{
    auto bytes = password.toUtf8();

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hashLen = 32;

    int result = PKCS5_PBKDF2_HMAC(
        bytes.constData(),
        bytes.size(),
        reinterpret_cast<const unsigned char*>(salt.constData()), salt.size(),
        iterations,
        EVP_sha256(),
        EVP_MAX_MD_SIZE,
        hash);

    if (result != 1) {
        qWarning() << "Password hash failed!";
        return QByteArray();
    }

    return QByteArray(reinterpret_cast<char *>(hash), hashLen);
}

QByteArray PasswordHasher::generateSalt(int length)
{
    QByteArray salt(length, '\0');
    if (RAND_bytes(reinterpret_cast<unsigned char*>(salt.data()), length) != 1) {
        qWarning() << "Failed to generate random salt!";
        return QByteArray();
    }
    return salt;
}
