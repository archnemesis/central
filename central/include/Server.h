//
// Created by robin on 11/22/2024.
//

#ifndef CENTRALSERVER_H
#define CENTRALSERVER_H

#include <QObject>
#include <QTcpServer>


namespace Central
{
    class ClientConnection;

    class Q_DECL_EXPORT Server : public QObject
    {
        Q_OBJECT
    public:
        explicit Server(QObject *parent = nullptr);

        void setListenAddress(const QHostAddress &address);
        void setListenPort(int port);

    protected:
        QList<ClientConnection*> m_clients;
        QTcpServer m_tcpServer;
        QHostAddress m_ipAddress;
        int m_port;

        protected slots:
            void newConnection();
        void onClientConnectionHelloReceived(QString message);
    };
}

#endif //CENTRALSERVER_H
