// Copyright 2024 Accenture.

#ifndef GUARD_2CF80A7A_1325_4C01_9F89_C3E9C64D2D5A
#define GUARD_2CF80A7A_1325_4C01_9F89_C3E9C64D2D5A

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

#endif /* GUARD_2CF80A7A_1325_4C01_9F89_C3E9C64D2D5A */
