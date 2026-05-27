/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

/**
 * \ingroup doip
 */
#pragma once

#include "doip/server/IDoIpServerSocketHandler.h"

#include <gmock/gmock.h>

namespace doip
{
/**
 * Mock class for DoIP socket handler.
 */
class DoIpServerSocketHandlerMock : public IDoIpServerSocketHandler
{
public:
    MOCK_METHOD1(start, void(IDoIpServerSocketHandlerListener&));
    MOCK_METHOD0(stop, void());
    MOCK_METHOD0(acquireSocket, ::tcp::AbstractSocket*());
    MOCK_METHOD2(
        releaseSocket, void(::tcp::AbstractSocket& socket, DoIpTcpConnection::ConnectionType type));
};

} // namespace doip
