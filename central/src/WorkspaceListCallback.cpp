//
// Created by robin on 11/23/2024.
//

#include "WorkspaceListCallback.h"

Central::WorkspaceListCallback::WorkspaceListCallback(
    const std::function<void(const QList<Workspace>& workspaces)>& onSuccess,
    const std::function<void(const Central::ProtocolError& error)>& onFailure,
    QObject* parent) :
        Callback(parent),
        m_onSuccess(onSuccess),
        m_onFailure(onFailure)
{

}

void Central::WorkspaceListCallback::onSuccess(const QList<Workspace>& workspaces)
{
    m_onSuccess(workspaces);
}

void Central::WorkspaceListCallback::onFailure(const Central::ProtocolError& error)
{
    m_onFailure(error);
}
