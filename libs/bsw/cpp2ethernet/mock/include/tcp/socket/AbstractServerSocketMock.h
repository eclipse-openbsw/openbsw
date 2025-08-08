// Copyright 2025 Accenture.

#pragma once

#include "tcp/socket/AbstractServerSocket.h"
#include "tcp/socket/AbstractSocket.h"
#include "tcp/socket/ISocketProvidingConnectionListener.h"

#include <gmock/gmock.h>

namespace tcp
{
struct AbstractServerSocketMock : public AbstractServerSocket
{
    virtual ~AbstractServerSocketMock() = default;

    AbstractServerSocketMock() : AbstractServerSocket() {}

    AbstractServerSocketMock(uint16_t port, ISocketProvidingConnectionListener& listener)
    : AbstractServerSocket(port, listener)
    {}

    MOCK_METHOD0(accept, bool());
    MOCK_METHOD2(bind, bool(::ip::IPAddress const&, uint16_t));
    MOCK_METHOD0(close, void());
    MOCK_CONST_METHOD0(isClosed, bool());

    ::tcp::AbstractSocket* connectFrom(::ip::IPAddress const& clientIpAddress, uint16_t clientPort)
    {
        if (_socketProvidingConnectionListener != nullptr)
        {
            ::tcp::AbstractSocket* socket
                = getSocketProvidingConnectionListener().getSocket(clientIpAddress, clientPort);
            if (socket != nullptr)
            {
                getSocketProvidingConnectionListener().connectionAccepted(*socket);
                return socket;
            }
        }
        return nullptr;
    }
};

} // namespace tcp
