// Copyright 2025 Accenture.

#pragma once

#include "tcp/socket/AbstractSocket.h"
#include "tcp/socket/ISocketProvidingConnectionListener.h"

#include <gmock/gmock.h>

namespace tcp
{
struct SocketProvidingConnectionListenerMock : public ISocketProvidingConnectionListener
{
    MOCK_METHOD2(getSocket, AbstractSocket*(ip::IPAddress const&, uint16_t));
    MOCK_METHOD1(connectionAccepted, void(AbstractSocket&));
};

} // namespace tcp
