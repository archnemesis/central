//
// Created by robin on 11/22/2024.
//

#include "Server.h"
#include "ClientConnection.h"

#include <QDebug>
#include <QTcpSocket>
#include <QThread>


Central::Server::Server(QObject* parent) :
    QObject(parent),
    m_ipAddress(QHostAddress::Any),
    m_port(1234)
{
    if (m_tcpServer.listen(m_ipAddress, m_port)) {
        qDebug() << "Listening on" << m_ipAddress << m_port;
        connect(&m_tcpServer,
            &QTcpServer::newConnection,
            this,
            &Server::newConnection);
    }
}

void Central::Server::setListenAddress(const QHostAddress& address)
{
    m_ipAddress = address;
}

void Central::Server::setListenPort(int port)
{
    m_port = port;
}

void Central::Server::newConnection()
{
    auto *socket = m_tcpServer.nextPendingConnection();
    auto *connection = new ClientConnection(socket);
    m_clients.append(connection);
    qDebug() << "New connection from" << socket->peerAddress();

    auto *thread = new QThread();
    connection->moveToThread(thread);
    connect(thread,
        &QThread::started,
        connection,
        &ClientConnection::run);
    connect(thread,
        &QThread::finished,
        thread,
        &QThread::deleteLater);
    connect(thread,
        &QThread::finished,
        connection,
        &QObject::deleteLater);

    // todo: connect application-specific signals

    connect(connection,
        &ClientConnection::helloReceived,
        this,
        &Central::Server::onClientConnectionHelloReceived);

    thread->start();
}

void Central::Server::onClientConnectionHelloReceived(QString message)
{
    qDebug() << "Got hello message:" << message;
}
