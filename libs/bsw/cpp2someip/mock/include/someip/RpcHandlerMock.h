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

#include "someip/IRpcHandler.h"
#include "someip/SomeIpMessage.h"

#include <gmock/gmock.h>

namespace someip
{
class RpcHandlerMock : public IRpcHandler
{
public:
    MOCK_METHOD(
        ErrorCode, handleMessage, (NetworkChannel const& channel, SomeIpMessage const& message));

    MOCK_METHOD(
        ErrorCode,
        handleRequest,
        (SomeIpMessage const& message,
         ::ip::IPEndpoint const& sourceAddress,
         uint16_t localPort,
         uint8_t proto));

    MOCK_METHOD(
        ErrorCode,
        handleResponse,
        (SomeIpMessage const& message, ::ip::IPEndpoint const& sourceAddress, uint16_t localPort));

    MOCK_METHOD(
        void,
        handleNotification,
        (SomeIpMessage const& message, ::ip::IPEndpoint const& sourceAddress, uint16_t localPort));

    MOCK_METHOD(
        ErrorCode,
        handleError,
        (SomeIpMessage const& message, ::ip::IPEndpoint const& sourceAddress));

    MOCK_METHOD(void, setEventReceiver, (IEventReceiver & eventReceiver));

    MOCK_METHOD(void, removeEventReceiver, ());
};

} // namespace someip
