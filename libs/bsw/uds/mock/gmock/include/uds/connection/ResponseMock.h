// Copyright 2024 Accenture.

#pragma once

#include "StubMock.h"
#include "uds/connection/PositiveResponse.h"

#include <gmock/gmock.h>

namespace uds
{
class PositiveResponseMockHelper
: public StubMock
, public PositiveResponse
{
public:
    PositiveResponseMockHelper() : StubMock() {}

    MOCK_METHOD(size_t, appendData, (uint8_t const[], size_t));
    MOCK_METHOD(uint8_t*, getData, ());
    MOCK_METHOD(size_t, getLength, ());

    static PositiveResponseMockHelper& instance()
    {
        static PositiveResponseMockHelper instance;
        return instance;
    }
};

} // namespace uds
