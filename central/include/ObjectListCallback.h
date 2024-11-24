//
// Created by robin on 11/23/2024.
//

#ifndef OBJECTLISTCALLBACK_H
#define OBJECTLISTCALLBACK_H

#include "Callback.h"
#include "Central.h"


namespace Central
{
    class ObjectListCallback : public Callback
    {
        Q_OBJECT
    public:
        ObjectListCallback(
            const std::function<void(const QList<Object>& objects)>& onSuccess,
            const std::function<void(const ProtocolError& error)>& onFailure,
            const std::function<void(int current, int total)>& onDownloadStatus,
            QObject *parent = nullptr);
        void onSuccess(const QList<Object>& objects);
        void onFailure(const ProtocolError& error);
        void onDownloadStatus(int current, int total);

    protected:
        std::function<void(const QList<Object>& layers)> m_onSuccess;
        std::function<void(const ProtocolError& error)> m_onFailure;
        std::function<void(int current, int total)> m_onDownloadStatus;
    };
}

#endif //OBJECTLISTCALLBACK_H
