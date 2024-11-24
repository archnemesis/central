//
// Created by robin on 11/23/2024.
//

#ifndef LAYERLISTCALLBACK_H
#define LAYERLISTCALLBACK_H

#include "Central.h"
#include "Callback.h"

namespace Central
{
    class LayerListCallback : public Callback
    {
        Q_OBJECT
    public:
        explicit LayerListCallback(
            const std::function<void(const QList<Layer>& layers)>& onSuccess,
            const std::function<void(const ProtocolError& error)>& onFailure,
            QObject *parent = nullptr);
        void onSuccess(const QList<Layer>& layers);
        void onFailure(const ProtocolError& error);

    protected:
        std::function<void(const QList<Layer>& layers)> m_onSuccess;
        std::function<void(const ProtocolError& error)> m_onFailure;
    };
}

#endif //LAYERLISTCALLBACK_H
