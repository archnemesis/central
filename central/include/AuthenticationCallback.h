//
// Created by robin on 11/23/2024.
//

#ifndef CENTRALAUTHENTICATIONCALLBACK_H
#define CENTRALAUTHENTICATIONCALLBACK_H

#include "Central.h"
#include "Callback.h"

namespace Central
{
    class AuthenticationCallback : public Callback
    {
    public:
        explicit AuthenticationCallback(
            const std::function<void(void)>& onSuccess,
            const std::function<void(const Central::ProtocolError& error)>& onFailure,
            QObject *parent = nullptr);
        void onSuccess();
        void onFailure(const Central::ProtocolError& error);

    protected:
        std::function<void(void)> m_onSuccess;
        std::function<void(const Central::ProtocolError& error)> m_onFailure;
    };
}

#endif //CENTRALAUTHENTICATIONCALLBACK_H
