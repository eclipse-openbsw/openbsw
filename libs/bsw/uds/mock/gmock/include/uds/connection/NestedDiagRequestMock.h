// Copyright 2024 Accenture.

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

    MOCK_CONST_METHOD1(getStoredRequestLength, uint16_t(::etl::span<uint8_t const> const& request));
    MOCK_CONST_METHOD2(
        storeRequest, void(::etl::span<uint8_t const> const& request, ::etl::span<uint8_t> dest));
    MOCK_METHOD1(
        prepareNestedRequest,
        ::etl::span<uint8_t const>(::etl::span<uint8_t const> const& storedRequest));
    MOCK_METHOD3(
        processNestedRequest,
        DiagReturnCode::Type(
            IncomingDiagConnection& connection, uint8_t const request[], uint16_t requestLength));
    MOCK_METHOD1(handleNestedResponseCode, void(DiagReturnCode::Type responseCode));
    MOCK_METHOD0(handleOverflow, void());
};

} // namespace uds
