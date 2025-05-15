// Copyright 2025 Accenture.

#pragma once

#include "tcp/socket/AbstractSocket.h"
#include "tcp/socket/ISocketProvidingConnectionListener.h"

#include <gmock/gmock.h>

namespace tcp
{
struct SocketProvidingConnectionListenerMock : public ISocketProvidingConnectionListener
{
    MOCK_METHOD(AbstractSocket*, getSocket, (ip::IPAddress const&, uint16_t));
    MOCK_METHOD(void, connectionAccepted, (AbstractSocket&));
};

} // namespace tcp
