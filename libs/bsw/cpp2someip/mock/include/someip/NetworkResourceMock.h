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

#include "someip/NetworkResource.h"

#include <etl/array.h>

#include <gmock/gmock.h>

namespace someip
{
class NetworkResourceMock : public NetworkResource
{
public:
    NetworkResourceMock() : buffer()
    {
        setInputBuffer(buffer);
        setOutputBuffer(buffer);
    }

    ~NetworkResourceMock() override = default;

    bool isInitialized() const override { return true; }

    MOCK_METHOD((::etl::expected<uint16_t, PortError>), getLocalPort, (), (const, override));
    MOCK_METHOD(uint8_t, getProto, (), (const, override));

    MOCK_METHOD(bool, isOpen, (), (const, override));
    MOCK_METHOD(bool, isConnected, (), (const, override));
    MOCK_METHOD(void, close, (), (override));

    MOCK_METHOD(bool, send, (::ip::IPEndpoint const& remoteEndpoint, uint32_t length), (override));
    MOCK_METHOD(bool, send, (uint32_t length), (override));

    static uint16_t const BUFFER_SIZE = 1500;
    ::etl::array<uint8_t, BUFFER_SIZE> buffer;
};

} // namespace someip
