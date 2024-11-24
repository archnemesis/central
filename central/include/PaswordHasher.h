//
// Created by robin on 11/22/2024.
//

#ifndef PASWORDHASHER_H
#define PASWORDHASHER_H

#include <QByteArray>


class PasswordHasher
{
public:
    static QByteArray hashPassword(const QString& password, const QString& salt, int iterations = 10000);
    static QByteArray generateSalt(int length = 16);
};

#endif //PASWORDHASHER_H
