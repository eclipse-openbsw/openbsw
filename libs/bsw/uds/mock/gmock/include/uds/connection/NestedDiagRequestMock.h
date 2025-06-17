// Copyright 2024 Accenture.

#pragma once

#include "uds/connection/NestedDiagRequest.h"

#include <gmock/gmock.h>

namespace uds
{
class NestedDiagRequestMock : public NestedDiagRequest
{
public:
    NestedDiagRequestMock(uint8_t prefixLength) : NestedDiagRequest(prefixLength) {}

    MOCK_METHOD(
        uint16_t, getStoredRequestLength, (::estd::slice<uint8_t const> const& request), (const));
    MOCK_METHOD(
        void,
        storeRequest,
        (::estd::slice<uint8_t const> const& request, ::estd::slice<uint8_t> dest),
        (const));
    MOCK_METHOD(
        ::estd::slice<uint8_t const>,
        prepareNestedRequest,
        (::estd::slice<uint8_t const> const& storedRequest));
    MOCK_METHOD(
        DiagReturnCode::Type,
        processNestedRequest,
        (IncomingDiagConnection & connection, uint8_t const request[], uint16_t requestLength));
    MOCK_METHOD(void, handleNestedResponseCode, (DiagReturnCode::Type responseCode));
    MOCK_METHOD(void, handleOverflow, ());
};

} // namespace uds
