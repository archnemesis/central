//
// Created by robin on 11/23/2024.
//

#include "LayerListCallback.h"

Central::LayerListCallback::LayerListCallback(
    const std::function<void(const QList<Layer>& layers)>& onSuccess,
    const std::function<void(const ProtocolError& error)>& onFailure,
    QObject* parent) : Callback(parent), m_onSuccess(onSuccess), m_onFailure(onFailure)
{

}

void Central::LayerListCallback::onSuccess(const QList<Layer>& layers)
{
    m_onSuccess(layers);
}

void Central::LayerListCallback::onFailure(const ProtocolError& error)
{
    m_onFailure(error);
}
