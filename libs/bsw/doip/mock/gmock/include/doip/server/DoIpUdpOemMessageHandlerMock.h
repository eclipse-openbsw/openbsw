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

#include "doip/server/IDoIpUdpOemMessageHandler.h"

#include <gmock/gmock.h>

namespace doip
{

class DoIpUdpOemMessageHandlerMock : public IDoIpUdpOemMessageHandler
{
public:
    MOCK_CONST_METHOD1(createResponse, void(::etl::span<uint8_t> response));
    MOCK_CONST_METHOD0(getRequestPayloadType, uint16_t(void));
    MOCK_CONST_METHOD0(getRequestPayloadSize, uint8_t(void));
    MOCK_CONST_METHOD0(getResponsePayloadType, uint16_t(void));
    MOCK_CONST_METHOD0(getResponsePayloadSize, uint8_t(void));
    MOCK_CONST_METHOD2(
        onPayloadReceived, bool(::etl::span<uint8_t const> payload, uint8_t& nackCode));
};
} // namespace doip
