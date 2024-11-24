//
// Created by robin on 11/22/2024.
//

#ifndef TESTCLIENT_H
#define TESTCLIENT_H

#include <QHostAddress>
#include <QObject>
#include "Central.h"
class QDataStream;
class QTcpSocket;


namespace Central
{
    class Callback;

    class Client : public QObject
    {
        Q_OBJECT
    public:
        explicit Client(const QHostAddress& address, int port, QObject *parent = nullptr);
        void connectToServer();
        void disconnectFromServer();
        void authenticate(
            const QString& username,
            const QString& password,
            const std::function<void()>& onSuccess,
            const std::function<void(const ProtocolError& error)>& onFailure);
        void getWorkspaces(
            const std::function<void(const QList<Workspace>& workspaces)>& onSuccess,
            const std::function<void(const ProtocolError& error)>& onFailure);
        void getLayersForWorkspace(
            const QUuid& workspaceId,
            const std::function<void(const QList<Layer>& layers)>& onSuccess,
            const std::function<void(const ProtocolError& error)>& onFailure);
        void getObjectsForLayer(
            const QUuid& layerId,
            const std::function<void(const QList<Object>& objects)>& onSuccess,
            const std::function<void(const ProtocolError& error)>& onFailure,
            const std::function<void(int current, int total)>& onDownloadStatus);

    protected:
        QTcpSocket *m_socket;
        QDataStream *m_stream;
        QHostAddress m_address;
        int m_port;
        QMap<int, Callback *> m_callbacks;
        bool m_isAuthenticated;

        bool checkForAuthentication(const ProtocolError& error);

        protected slots:
            void onReadyRead();
        void onConnected();
        void onDisconnected();

    signals:
        void helloRequestReceived(QString message);
        void objectDownloadStarted(int total);
        void objectDownloadStatus(int current);
        void objectDownloadFinished();
    };
}


#endif //TESTCLIENT_H
