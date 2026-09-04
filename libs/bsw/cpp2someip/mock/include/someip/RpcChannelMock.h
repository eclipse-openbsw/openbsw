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

#include "someip/IRpcChannel.h"
#include "someip/ISomeIpSerializable.h"

#include <ip/IPEndpoint.h>

#include <gmock/gmock.h>

namespace someip
{
class RpcChannelMock : public IRpcChannel
{
public:
    MOCK_METHOD(uint16_t, getServiceId, (), (const));
    MOCK_METHOD(uint16_t, getClientId, (), (const));
    MOCK_METHOD(uint16_t, getSessionId, (), (const));
    MOCK_METHOD(void, setSessionId, (uint16_t));

    MOCK_METHOD(::ip::IPEndpoint const&, getRemoteIp, (), (const));

    MOCK_METHOD((::etl::expected<uint16_t, PortError>), getLocalPort, (), (const, override));
    MOCK_METHOD(uint8_t, getProto, (), (const));

    MOCK_METHOD(void, cancelTimeout, (), (override));
    MOCK_METHOD(
        void, setTimeout, (::async::ContextType const context, uint32_t timeout), (override));

    MOCK_METHOD(
        ServiceResultCode,
        callMethod,
        (uint16_t methodId,
         ISomeIpSerializable const* pRequest,
         uint8_t interfaceVersion,
         ISomeIpSerializable* pResponse,
         CallDoneClosure& done,
         uint32_t timeout));

    MOCK_METHOD(
        ServiceResultCode,
        callFireAndForgetMethod,
        (uint16_t methodId, ISomeIpSerializable const* pRequest, uint8_t interfaceVersion));

    MOCK_METHOD(ISomeIpSerializable*, getResponse, ());

    MOCK_METHOD(void, responseReceived, (ServiceResultCode result));
};

} // namespace someip
