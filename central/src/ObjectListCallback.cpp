//
// Created by robin on 11/23/2024.
//

#include "ObjectListCallback.h"

#include "Callback.h"

Central::ObjectListCallback::ObjectListCallback(
    const std::function<void(const QList<Object>& objects)>& onSuccess,
    const std::function<void(const ProtocolError& error)>& onFailure,
    const std::function<void(int current, int total)>& onDownloadStatus,
    QObject *parent) :
        Callback(parent),
        m_onSuccess(onSuccess),
        m_onFailure(onFailure),
        m_onDownloadStatus(onDownloadStatus)

{

}

void Central::ObjectListCallback::onSuccess(const QList<Object>& objects)
{
    m_onSuccess(objects);
}

void Central::ObjectListCallback::onFailure(const ProtocolError& error)
{
    m_onFailure(error);
}

void Central::ObjectListCallback::onDownloadStatus(int current, int total)
{
    m_onDownloadStatus(current, total);
}
