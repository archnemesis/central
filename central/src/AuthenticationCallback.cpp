//
// Created by robin on 11/23/2024.
//

#include "AuthenticationCallback.h"

Central::AuthenticationCallback::AuthenticationCallback(
        const std::function<void(void)>& onSuccess,
        const std::function<void(const Central::ProtocolError& error)>& onFailure,
        QObject *parent) :
    Callback(parent),
    m_onSuccess(onSuccess),
    m_onFailure(onFailure)
{

}

void Central::AuthenticationCallback::onSuccess()
{
    m_onSuccess();
}

void Central::AuthenticationCallback::onFailure(const Central::ProtocolError& error)
{
    m_onFailure(error);
}
