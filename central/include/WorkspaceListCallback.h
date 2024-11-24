//
// Created by robin on 11/23/2024.
//

#ifndef CENTRALWORKSPACELISTCALLBACK_H
#define CENTRALWORKSPACELISTCALLBACK_H

#include "Central.h"
#include "Callback.h"

namespace Central
{
    class WorkspaceListCallback : public Callback
    {
        Q_OBJECT
    public:
        explicit WorkspaceListCallback(
            const std::function<void(const QList<Workspace>& workspaces)>& onSuccess,
            const std::function<void(const ProtocolError& error)>& onFailure,
            QObject *parent = nullptr);
        void onSuccess(const QList<Workspace>& workspaces);
        void onFailure(const ProtocolError& error);

    protected:
        std::function<void(const QList<Workspace>& workspaces)> m_onSuccess;
        std::function<void(const ProtocolError& error)> m_onFailure;
    };
}

#endif //CENTRALWORKSPACELISTCALLBACK_H
