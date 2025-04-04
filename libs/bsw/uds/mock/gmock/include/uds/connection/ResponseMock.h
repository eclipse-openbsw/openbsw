// Copyright 2024 Accenture.

#pragma once

#include "StubMock.h"
#include "uds/base/AbstractDiagJob.h"
#include "uds/connection/PositiveResponse.h"

#include <gmock/gmock.h>

namespace uds
{
class PositiveResponseMockHelper
: public StubMock
, public PositiveResponse
{
public:
    PositiveResponseMockHelper(IncomingDiagConnection& connection) : StubMock() {}

    MOCK_METHOD(size_t, appendData, (uint8_t const[], size_t));
    MOCK_METHOD(uint8_t*, getData, ());
    MOCK_METHOD(::uds::ErrorCode, send, (AbstractDiagJob&));

    static PositiveResponseMockHelper& instance(IncomingDiagConnection& connection)
    {
        static PositiveResponseMockHelper instance(connection);
        return instance;
    }
};

} // namespace uds
