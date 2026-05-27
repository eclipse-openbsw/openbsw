/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "uds/connection/NestedDiagRequest.h"

#include <etl/span.h>

#include <gmock/gmock.h>

namespace uds
{
class NestedDiagRequestMock : public NestedDiagRequest
{
public:
    NestedDiagRequestMock(uint8_t prefixLength) : NestedDiagRequest(prefixLength) {}

    MOCK_METHOD(
        uint16_t,
        getStoredRequestLength,
        (::etl::span<uint8_t const> const& request),
        (const, override));
    MOCK_METHOD(
        void,
        storeRequest,
        (::etl::span<uint8_t const> const& request, ::etl::span<uint8_t> dest),
        (const, override));
    MOCK_METHOD(
        ::etl::span<uint8_t const>,
        prepareNestedRequest,
        (::etl::span<uint8_t const> const& storedRequest),
        (override));
    MOCK_METHOD(
        DiagReturnCode::Type,
        processNestedRequest,
        (IncomingDiagConnection & connection, uint8_t const request[], uint16_t requestLength),
        (override));
    MOCK_METHOD(void, handleNestedResponseCode, (DiagReturnCode::Type responseCode), (override));
    MOCK_METHOD(void, handleOverflow, (), (override));
};

} // namespace uds
