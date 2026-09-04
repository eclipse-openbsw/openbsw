/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "someip/TcpProxy.h"

#include <gmock/gmock.h>

namespace someip
{
class TcpProxyMock : public TcpProxy
{
public:
    TcpProxyMock(::tcp::AbstractSocket& socket) : TcpProxy(socket) {}

    MOCK_METHOD(bool, isInitialized, (), (const));

    MOCK_METHOD(bool, isOpen, (), (const));
    MOCK_METHOD(bool, isConnected, (), (const));

    MOCK_METHOD(void, close, ());

    MOCK_METHOD(bool, send, (::ip::IPEndpoint const& remoteEndpoint, uint32_t length));
    MOCK_METHOD(bool, send, (uint32_t length));

    MOCK_METHOD(void, dataReceived, (uint16_t length));

    MOCK_METHOD(void, connectionClosed, (::tcp::IDataListener::ErrorCode status));
};

class TcpProxyConnectionListenerMock : public TcpProxy::IConnectionListener
{
public:
    MOCK_METHOD(void, connectionChanged, (TcpProxy & proxy));
};

} // namespace someip
