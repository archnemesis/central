//
// Created by robin on 11/22/2024.
//

#ifndef CENTRALCLIENTCONNECTION_H
#define CENTRALCLIENTCONNECTION_H

#include <QObject>
#include <QUuid>

#include "Central.h"


class QTcpSocket;

namespace Central
{
    class Q_DECL_EXPORT ClientConnection : public QObject
    {
        Q_OBJECT
    public:
        explicit ClientConnection(QTcpSocket *socket, QObject *parent = nullptr);

    public slots:
        void run();

    protected:
        QTcpSocket *m_socket;
        bool m_isAuthenticated = false;
        QUuid m_userId;
        int m_sequenceNumber;

        void sendHello();
        void sendErrorResponse(
            quint32 sequenceNumber,
            quint32 packetType,
            const Central::ProtocolError& error);

        signals:
            void helloReceived(QString message);
        void authenticationRequested();
        void authenticationAccepted();
        void authenticationRejected();
    };
}

#endif //CENTRALCLIENTCONNECTION_H
