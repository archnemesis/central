//
// Created by robin on 11/22/2024.
//

#ifndef CENTRAL_H
#define CENTRAL_H

#define ERROR_OK 0
#define ERROR_DATABASE 1

#define PACKET_TYPE_HELLO_REQUEST 1
#define PACKET_TYPE_HELLO_RESPONSE 2
#define PACKET_TYPE_AUTHENTICATION_REQUEST 3
#define PACKET_TYPE_AUTHENTICATION_RESPONSE 4
#define PACKET_TYPE_WORKSPACE_LIST_REQUEST 5
#define PACKET_TYPE_WORKSPACE_LIST_RESPONSE 6
#define PACKET_TYPE_LAYER_LIST_REQUEST 7
#define PACKET_TYPE_LAYER_LIST_RESPONSE 8
#define PACKET_TYPE_OBJECT_LIST_REQUEST 9
#define PACKET_TYPE_OBJECT_LIST_RESPONSE 10

#define AUTHENTICATION_ERROR_SUCCESS 0
#define AUTHENTICATION_ERROR_ACCOUNT_NOT_FOUND 1
#define AUTHENTICATION_ERROR_INVALID_PASSWORD 2
#define AUTHENTICATION_ERROR_ACCOUNT_LOCKED 3
#define AUTHENTICATION_ERROR_OTHER 4
#include <QDateTime>
#include <QUuid>

namespace Central
{
    enum Command
    {
        HelloCommand,
        AuthenticateCommand,
        GetLayersForWorkspaceCommand,
        GetObjectsForLayerCommand,
        GetObjectsForViewportCommand
    };

    enum PacketType
    {
        HelloRequest = 1,
        HelloResponse,
        AuthenticationRequest,
        AuthenticationResponse,
        WorkspaceListRequest,
        WorkspaceListResponse
    };

    enum ProtocolError
    {
        NoError,
        AccountNotFound,
        InvalidPassword,
        AccountLocked,
        DatabaseError,
        NotAuthenticatedError,
        Other
    };

    struct Workspace
    {
        QUuid id;
        QString name;
        QString description;
        QDateTime created;
        QDateTime updated;
        QUuid owner;
    };

    struct Layer
    {
        QUuid id;
        QUuid workspaceId;
        QUuid createdBy;
        QUuid lastEditedBy;
        QDateTime created;
        QDateTime updated;
        QString name;
        QString description;
        quint32 color;
        quint32 order;
    };

    struct Object
    {
        QUuid id;
        QUuid layerId;
        QUuid createdBy;
        QUuid lastEditedBy;
        QString type;
        QDateTime created;
        QDateTime updated;
        QString label;
        QString description;
        QByteArray data;
        QString geom;
    };
}


#endif //CENTRAL_H
