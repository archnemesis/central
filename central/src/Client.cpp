//
// Created by robin on 11/22/2024.
//

#include "Client.h"
#include "Central.h"
#include "Callback.h"

#include <QDataStream>
#include <QTcpSocket>
#include <QRandomGenerator>
#include <QUuid>

#include "AuthenticationCallback.h"
#include "LayerListCallback.h"
#include "ObjectListCallback.h"
#include "WorkspaceListCallback.h"
#include "WorkspaceObject.h"

Central::Client::Client(const QHostAddress& address, int port, QObject* parent) :
    QObject(parent),
    m_address(address),
    m_port(port),
    m_isAuthenticated(false)
{
    m_socket = new QTcpSocket(this);
    m_stream = new QDataStream(m_socket);

    connect(m_socket,
        &QTcpSocket::readyRead,
        this,
        &Client::onReadyRead);
    connect(m_socket,
        &QTcpSocket::disconnected,
        this,
        &Client::onDisconnected);
}

void Central::Client::connectToServer()
{
    m_socket->connectToHost(m_address, m_port);
}

void Central::Client::disconnectFromServer()
{
    m_socket->disconnectFromHost();
}

void Central::Client::authenticate(
        const QString& username,
        const QString& password,
        const std::function<void()>& onSuccess,
        const std::function<void(const ProtocolError& error)>& onFailure)
{
    auto *authCallback = new AuthenticationCallback(
        onSuccess,
        onFailure);

    int sequenceNumber = QRandomGenerator::global()->bounded(10240);
    m_callbacks[sequenceNumber] = authCallback;

    QDataStream stream(m_socket);
    stream.setVersion(QDataStream::Qt_5_15);
    stream << (quint32)PACKET_TYPE_AUTHENTICATION_REQUEST;
    stream << (quint32)sequenceNumber;
    stream << username;
    stream << password;
    m_socket->flush();
}

void Central::Client::getWorkspaces(
    const std::function<void(const QList<Workspace>& workspaces)>& onSuccess,
    const std::function<void(const ProtocolError& error)>& onFailure)
{
    auto *workspaceListCallback = new WorkspaceListCallback(
        onSuccess,
        onFailure);

    int sequenceNumber = QRandomGenerator::global()->bounded(10240);
    m_callbacks[sequenceNumber] = workspaceListCallback;

    QDataStream stream(m_socket);
    stream.setVersion(QDataStream::Qt_5_15);
    stream << static_cast<quint32>(PACKET_TYPE_WORKSPACE_LIST_REQUEST);
    stream << static_cast<quint32>(sequenceNumber);
    m_socket->flush();
}

void Central::Client::getLayersForWorkspace(
    const QUuid& workspaceId,
    const std::function<void(const QList<Layer>& layers)>& onSuccess,
    const std::function<void(const ProtocolError& error)>& onFailure)
{
    auto *layerListCallback = new LayerListCallback(
        onSuccess,
        onFailure);

    int sequenceNumber = QRandomGenerator::global()->bounded(10240);
    m_callbacks[sequenceNumber] = layerListCallback;

    QDataStream stream(m_socket);
    stream.setVersion(QDataStream::Qt_5_15);
    stream << static_cast<quint32>(PACKET_TYPE_LAYER_LIST_REQUEST);
    stream << static_cast<quint32>(sequenceNumber);
    stream << workspaceId;
    m_socket->flush();
}

void Central::Client::getObjectsForLayer(
    const QUuid& layerId,
    const std::function<void(const QList<Object>& objects)>& onSuccess,
    const std::function<void(const ProtocolError& error)>& onFailure,
    const std::function<void(int current, int total)>& onDownloadStatus)
{
    auto *objectListCallback = new ObjectListCallback(
        onSuccess,
        onFailure,
        onDownloadStatus);

    int sequenceNumber = QRandomGenerator::global()->bounded(10240);
    m_callbacks[sequenceNumber] = objectListCallback;

    QDataStream stream(m_socket);
    stream.setVersion(QDataStream::Qt_5_15);
    stream << static_cast<quint32>(PACKET_TYPE_OBJECT_LIST_REQUEST);
    stream << static_cast<quint32>(sequenceNumber);
    stream << layerId;
    m_socket->flush();
}

bool Central::Client::checkForAuthentication(const ProtocolError& error)
{
    return true;
}

void Central::Client::onReadyRead()
{
    quint32 packetType = 0;
    quint32 sequenceNumber = 0;

    qDebug() << "processing" << m_socket->bytesAvailable() << "bytes";

    m_stream->startTransaction();

    m_stream->startTransaction();
    *m_stream >> packetType;
    *m_stream >> sequenceNumber;
    if (!m_stream->commitTransaction())
        return;

    switch (packetType) {
    case PACKET_TYPE_HELLO_REQUEST: {
        QString helloString;
        m_stream->startTransaction();
        *m_stream >> helloString;
        if (!m_stream->commitTransaction()) {
            break;
        }

        qInfo() << "hello request from server:" << helloString;

        QDataStream out(m_socket);
        out.setVersion(QDataStream::Qt_5_15);
        out << static_cast<quint32>(PACKET_TYPE_HELLO_RESPONSE);
        out << sequenceNumber;
        out << QString("Hello from client");

        emit helloRequestReceived(helloString);

        break;
    }
    case PACKET_TYPE_AUTHENTICATION_RESPONSE: {
        bool authenticated = false;
        quint32 error = 0;

        m_stream->startTransaction();
        *m_stream >> error;
        *m_stream >> authenticated;

        if (!m_stream->commitTransaction())
            break;

        m_isAuthenticated = authenticated;

        AuthenticationCallback *authenticationCallback = nullptr;
        if (const auto& keys = m_callbacks.keys(); keys.contains((int)sequenceNumber)) {
            auto *callback = m_callbacks.value((int)sequenceNumber);
            authenticationCallback = dynamic_cast<AuthenticationCallback *>(callback);

            if (authenticated)
                authenticationCallback->onSuccess();
            else
                authenticationCallback->onFailure(static_cast<ProtocolError>(error));

            delete authenticationCallback;
        }


        break;
    }
    case PACKET_TYPE_WORKSPACE_LIST_RESPONSE: {
        quint32 count = 0;
        quint32 error = 0;

        m_stream->startTransaction();
        *m_stream >> error;
        *m_stream >> count;

        if (!m_stream->commitTransaction()) {
            break;
        }

        WorkspaceListCallback *workspaceListCallback = nullptr;
        if (const auto& keys = m_callbacks.keys(); keys.contains((int)sequenceNumber)) {
            auto *callback = m_callbacks.value((int)sequenceNumber);
            workspaceListCallback = dynamic_cast<WorkspaceListCallback *>(callback);
        }

        if (error != NoError) {
            if (workspaceListCallback) {
                workspaceListCallback->onFailure(static_cast<ProtocolError>(error));
            }
            break;
        }

        QList<Workspace> workspaces;

        m_stream->startTransaction();
        for (int i = 0; i < count; i++) {
            QUuid workspaceId;
            QString workspaceName;
            QString workspaceDescription;
            QDateTime created;
            QDateTime updated;
            QUuid owner;

            *m_stream >> workspaceId;
            *m_stream >> created;
            *m_stream >> updated;
            *m_stream >> workspaceName;
            *m_stream >> workspaceDescription;
            *m_stream >> owner;

            Workspace workspace;
            workspace.id = workspaceId;
            workspace.created = created;
            workspace.updated = updated;
            workspace.name = workspaceName;
            workspace.description = workspaceDescription;
            workspace.owner = owner;

            workspaces.append(workspace);
        }

        if (!m_stream->commitTransaction())
            break;

        if (workspaceListCallback)
            workspaceListCallback->onSuccess(workspaces);

        delete workspaceListCallback;

        break;
    }
    case PACKET_TYPE_LAYER_LIST_RESPONSE: {
        quint32 error = 0;
        quint32 count = 0;

        m_stream->startTransaction();
        *m_stream >> error;
        *m_stream >> count;

        if (!m_stream->commitTransaction())
            break;

        LayerListCallback *layerListCallback = nullptr;
        if (const auto& keys = m_callbacks.keys(); keys.contains((int)sequenceNumber)) {
            auto *callback = m_callbacks.value((int)sequenceNumber);
            layerListCallback = dynamic_cast<LayerListCallback *>(callback);
        }

        if (error != NoError) {
            if (layerListCallback) {
                layerListCallback->onFailure(static_cast<ProtocolError>(error));
            }
            break;
        }

        QList<Layer> layers;

        m_stream->startTransaction();
        for (int i = 0; i < count; i++) {
            QUuid layerId;
            QUuid workspaceId;
            QUuid createdBy;
            QUuid lastEditedBy;
            QDateTime created;
            QDateTime updated;
            QString name;
            QString description;
            quint32 color;
            quint32 order;

            *m_stream >> layerId;
            *m_stream >> workspaceId;
            *m_stream >> createdBy;
            *m_stream >> lastEditedBy;
            *m_stream >> created;
            *m_stream >> updated;
            *m_stream >> name;
            *m_stream >> description;
            *m_stream >> color;
            *m_stream >> order;

            Layer layer;
            layer.id = layerId;
            layer.created = created;
            layer.lastEditedBy = lastEditedBy;
            layer.created = created;
            layer.updated = updated;
            layer.name = name;
            layer.description = description;
            layer.color = color;
            layer.order = order;

            layers.append(layer);
        }

        if (!m_stream->commitTransaction())
            break;

        if (layerListCallback)
            layerListCallback->onSuccess(layers);

        delete layerListCallback;

        break;
    }
    case PACKET_TYPE_OBJECT_LIST_RESPONSE: {
        quint32 error = 0;
        quint32 count = 0;

        m_stream->startTransaction();
        *m_stream >> error;
        *m_stream >> count;

        if (!m_stream->commitTransaction())
            break;

        ObjectListCallback *objectListCallback = nullptr;
        if (const auto& keys = m_callbacks.keys(); keys.contains((int)sequenceNumber)) {
            auto *callback = m_callbacks.value((int)sequenceNumber);
            objectListCallback = dynamic_cast<ObjectListCallback *>(callback);
        }

        if (error != NoError) {
            if (objectListCallback)
                objectListCallback->onFailure(static_cast<ProtocolError>(error));
            break;
        }

        QList<Object> objects;

        m_stream->startTransaction();
        for (int i = 0; i < count; i++) {
            if (objectListCallback)
                objectListCallback->onDownloadStatus(i + 1, count);

            QUuid objectId;
            QUuid layerId;
            QUuid createdBy;
            QUuid lastEditedBy;
            QString type;
            QDateTime created;
            QDateTime updated;
            QString label;
            QString description;
            QString geom;
            QByteArray data;

            *m_stream >> objectId;
            *m_stream >> layerId;
            *m_stream >> createdBy;
            *m_stream >> lastEditedBy;
            *m_stream >> type;
            *m_stream >> created;
            *m_stream >> updated;
            *m_stream >> label;
            *m_stream >> description;
            *m_stream >> geom;
            *m_stream >> data;

            Object object;
            object.id = objectId;
            object.layerId = layerId;
            object.createdBy = createdBy;
            object.lastEditedBy = lastEditedBy;
            object.type = type;
            object.created = created;
            object.updated = updated;
            object.label = label;
            object.description = description;
            object.geom = geom;
            object.data = data;
            objects.append(object);
        }

        if (!m_stream->commitTransaction())
            break;

        if (objectListCallback)
            objectListCallback->onSuccess(objects);

        delete objectListCallback;

        break;
    }
    }

    m_stream->commitTransaction();
}

void Central::Client::onConnected()
{
    qDebug() << "Connected";
}

void Central::Client::onDisconnected()
{
    qDebug() << "Disconnected";
}
