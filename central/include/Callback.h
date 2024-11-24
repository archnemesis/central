//
// Created by robin on 11/23/2024.
//

#ifndef CENTRALCALLBACK_H
#define CENTRALCALLBACK_H

#include <QObject>

namespace Central
{
    class Callback : public QObject
    {
        Q_OBJECT
    public:
        explicit Callback(QObject *parent = nullptr);
    };
}

#endif //CENTRALCALLBACK_H
