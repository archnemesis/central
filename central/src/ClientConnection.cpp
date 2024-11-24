//
// Created by robin on 11/22/2024.
//

#include "ClientConnection.h"
#include "Central.h"

#include <QDebug>
#include <QTcpSocket>
#include <QDataStream>
#include <QDateTime>
#include <QHostAddress>
#include <QRandomGenerator>
#include <QThread>
#include <QUuid>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

Central::ClientConnection::ClientConnection(QTcpSocket *socket, QObject* parent) :
    QObject(parent),
    m_socket(socket),
    m_sequenceNumber(0)
{
    sendHello();
}

void Central::ClientConnection::run()
{
    QDataStream data(m_socket);
    data.setVersion(QDataStream::Qt_5_15);

    qInfo() << "Connecting to database";

    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL7");

    db.setHostName("localhost");
    db.setPort(54321);
    db.setUserName("postgres");
    db.setPassword("password");
    db.setDatabaseName("postgres");

    if (!db.open()) {
        qCritical() << "Unable to connect to database server";
        qCritical() << db.lastError().text();
        return;
    }

    while (m_socket->state() == QAbstractSocket::ConnectedState) {
        if (m_socket->bytesAvailable() < sizeof(quint32)) {
            QThread::msleep(10);
            continue;
        }

        qDebug() << "processing" << m_socket->bytesAvailable() << "bytes";

        quint32 packetType;
        quint32 sequenceNumber;

        data.startTransaction();
        data >> packetType;
        data >> sequenceNumber;

        switch (packetType) {
        /*
         * Hello request
         * quint32  packet type
         * quint32  sequence number
         * QString  hello string
         */
        case PACKET_TYPE_HELLO_REQUEST: {
            QString message;

            data.startTransaction();
            data >> message;

            if (!data.commitTransaction())
                break;

            emit helloReceived(message);

            QDataStream out(m_socket);
            out.setVersion(QDataStream::Qt_5_15);
            out << (quint32)PACKET_TYPE_HELLO_RESPONSE;
            out << sequenceNumber;
            out << QString("hello from server");
            m_socket->flush();

            qInfo() << "hello request from" << m_socket->peerAddress().toString() << message;

            break;
        }
        case PACKET_TYPE_HELLO_RESPONSE: {
            QString message;

            data.startTransaction();
            data >> message;

            if (!data.commitTransaction()) {
                break;
            }

            qDebug() << "hello response from" << m_socket->peerAddress().toString() << message;
            break;
        }
        /*
         * Authentication request
         * quint32  packet type
         * quint32  sequence number
         * quint32  authentication error code
         * bool     request accepted/denied
         */
        case PACKET_TYPE_AUTHENTICATION_REQUEST: {
            QString username;
            QString password;
            Central::ProtocolError error = Central::NoError;

            data.startTransaction();
            data >> username;
            data >> password;
            if (!data.commitTransaction()) {
                break;;
            }

            QSqlQuery query;
            query.prepare("SELECT id, password FROM osmserver.users WHERE username = :username");
            query.bindValue(":username", username);

            if (!query.exec()) {
                qWarning() << "Unable to query database:" << query.lastError().text();
                error = Central::DatabaseError;
            }
            else {
                if (query.next()) {
                    QUuid dbUserId = query.value(0).toUuid();
                    QString dbPassword = query.value(1).toString();

                    if (password != dbPassword) {
                        error = Central::InvalidPassword;
                        qInfo() << "username:" << username << "password invalid";
                    }
                    else {
                        m_isAuthenticated = true;
                        m_userId = dbUserId;
                        qInfo() << "username:" << username << "successfully authenticated";
                    }
                }
                else {
                    error = AccountNotFound;
                    qInfo() << "username:" << username << "account not found";
                }
            }

            QDataStream out(m_socket);
            out.setVersion(QDataStream::Qt_5_15);
            out << (quint32)PACKET_TYPE_AUTHENTICATION_RESPONSE;
            out << sequenceNumber;
            out << (quint32)error;
            out << m_isAuthenticated;
            m_socket->flush();

            qDebug() << "Should have sent success message";

            break;
        }
        case PACKET_TYPE_WORKSPACE_LIST_REQUEST: {
            if (!m_isAuthenticated) {
                QDataStream out(m_socket);
                out.setVersion(QDataStream::Qt_5_15);
                out << static_cast<quint32>(PACKET_TYPE_WORKSPACE_LIST_RESPONSE);
                out << sequenceNumber;
                out << static_cast<quint32>(NotAuthenticatedError);
                out << static_cast<quint32>(0);
                m_socket->flush();
                break;
            }

            QSqlQuery query;
            query.prepare("SELECT id, name, description FROM osmserver.workspaces;");

            if (!query.exec()) {
                qWarning() << "Unable to query for workspaces:" << query.lastError().text();
                // todo: return error?
                break;
            }

            QDataStream out(m_socket);
            out.setVersion(QDataStream::Qt_5_15);
            out << (quint32)PACKET_TYPE_WORKSPACE_LIST_RESPONSE;
            out << sequenceNumber;
            out << (quint32)NoError;
            out << (quint32)query.size();

            while (query.next()) {
                QUuid workspaceId = query.value("id").toUuid();
                QDateTime created = query.value("created").toDateTime();
                QDateTime updated = query.value("updated").toDateTime();
                QString workspaceName = query.value(1).toString();
                QString workspaceDescription = query.value(2).toString();
                QUuid owner = query.value("owner").toUuid();

                out << workspaceId;
                out << created;
                out << updated;
                out << workspaceName;
                out << workspaceDescription;
                out << owner;
            }

            m_socket->flush();
            break;
        }
        case PACKET_TYPE_LAYER_LIST_REQUEST: {
            if (!m_isAuthenticated) {
                qWarning() << "User not authenticated";
                sendErrorResponse(
                    sequenceNumber,
                    PACKET_TYPE_LAYER_LIST_RESPONSE,
                    NotAuthenticatedError);
                break;
            }

            QUuid workspaceId;

            data.startTransaction();
            data >> workspaceId;
            if (!data.commitTransaction())
                break;

            QSqlQuery query;
            query.prepare("SELECT "
                "\"id\", \"name\", \"description\", \"color\", \"order\", "
                "\"created\", \"updated\", \"created_by\", \"last_edited_by\" "
                "FROM osmserver.layers "
                "WHERE \"workspace_id\" = :workspace_id;");

            query.bindValue(
                ":workspace_id",
                workspaceId.toString()
                    .mid(1, workspaceId.toString().length() - 2));

            if (!query.exec()) {
                qWarning() << "Unable to query for layers:" << query.lastError().text();
                sendErrorResponse(
                    sequenceNumber,
                    PACKET_TYPE_LAYER_LIST_RESPONSE,
                    DatabaseError);
                break;
            }

            QDataStream out(m_socket);
            out.setVersion(QDataStream::Qt_5_15);

            out << static_cast<quint32>(PACKET_TYPE_LAYER_LIST_RESPONSE);
            out << sequenceNumber;
            out << static_cast<quint32>(NoError);
            out << static_cast<quint32>(query.size());

            while (query.next()) {
                QUuid layerId = query.value("id").toUuid();
                QUuid createdBy = query.value("created_by").toUuid();
                QUuid lastEditedBy = query.value("last_edited_by").toUuid();
                QDateTime created = query.value("created").toDateTime();
                QDateTime updated = query.value("updated").toDateTime();
                QString name = query.value("name").toString();
                QString description = query.value("description").toString();
                quint32 color = query.value("color").toUInt();
                quint32 order = query.value("order").toUInt();

                qDebug() << "Processing layer" << name;

                out << layerId;
                out << workspaceId;
                out << createdBy;
                out << lastEditedBy;
                out << created;
                out << updated;
                out << name;
                out << description;
                out << color;
                out << order;
            }

            m_socket->flush();
            break;
        }
        case PACKET_TYPE_OBJECT_LIST_REQUEST: {
            if (!m_isAuthenticated) {
                qWarning() << "User not authenticated";
                sendErrorResponse(
                    sequenceNumber,
                    PACKET_TYPE_OBJECT_LIST_RESPONSE,
                    NotAuthenticatedError);
                break;
            }

            QUuid layerId;

            data.startTransaction();
            data >> layerId;
            if (!data.commitTransaction())
                break;

            QSqlQuery query;
            query.prepare("SELECT * FROM osmserver.objects WHERE layer_id = :layer_id");
            query.bindValue(
                ":layer_id",
                layerId.toString()
                    .mid(1, layerId.toString().length() - 2));

            if (!query.exec()) {
                qWarning() << "Unable to query for objects:" << query.lastError().text();
                sendErrorResponse(
                    sequenceNumber,
                    PACKET_TYPE_OBJECT_LIST_RESPONSE,
                    DatabaseError);
                break;
            }

            QDataStream out(m_socket);
            out.setVersion(QDataStream::Qt_5_15);

            out << static_cast<quint32>(PACKET_TYPE_OBJECT_LIST_RESPONSE);
            out << sequenceNumber;
            out << static_cast<quint32>(NoError);
            out << static_cast<quint32>(query.size());

            while (query.next()) {
                QUuid objectId = query.value("id").toUuid();
                QUuid layerId = query.value("layer").toUuid();
                QUuid createdBy = query.value("created_by").toUuid();
                QUuid lastEditedBy = query.value("last_edited_by").toUuid();
                QString type;
                QDateTime created = query.value("created").toDateTime();
                QDateTime updated = query.value("updated").toDateTime();
                QString label = query.value("label").toString();
                QString description = query.value("description").toString();
                QString geom = query.value("geom").toString();
                QByteArray data = query.value("data").toByteArray();

                out << objectId;
                out << layerId;
                out << createdBy;
                out << lastEditedBy;
                out << type;
                out << created;
                out << updated;
                out << label;
                out << description;
                out << geom;
                out << data;
            }

            m_socket->flush();
            break;
        }
        }

        data.commitTransaction();
    }

    qInfo() << "client disconnecting";
}

void Central::ClientConnection::sendHello()
{
    int sequenceNumber = QRandomGenerator::global()->bounded(101);
    QDataStream dataStream(m_socket);
    dataStream.setVersion(QDataStream::Qt_5_0);
    dataStream << static_cast<quint32>(PACKET_TYPE_HELLO_REQUEST);
    dataStream << static_cast<quint32>(sequenceNumber);
    dataStream << QString("Hello from server");
}

void Central::ClientConnection::sendErrorResponse(
    quint32 sequenceNumber,
    quint32 packetType,
    const Central::ProtocolError& error)
{
    QDataStream out(m_socket);
    out.setVersion(QDataStream::Qt_5_15);
    out << packetType;
    out << sequenceNumber;
    out << static_cast<quint32>(error);
    out << static_cast<quint32>(0);
    m_socket->flush();
}
