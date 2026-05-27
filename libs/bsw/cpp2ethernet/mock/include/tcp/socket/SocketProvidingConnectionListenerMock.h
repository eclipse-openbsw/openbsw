/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
