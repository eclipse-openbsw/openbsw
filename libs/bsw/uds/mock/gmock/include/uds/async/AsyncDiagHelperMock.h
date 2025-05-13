// Copyright 2024 Accenture.

#pragma once

#include "uds/async/IAsyncDiagHelper.h"

#include <gmock/gmock.h>

namespace uds
{
class AsyncDiagHelperMock : public IAsyncDiagHelper
{
public:
    MOCK_CONST_METHOD0(getDiagContext, ::async::ContextType());

    MOCK_METHOD3(
        allocateRequest,
        StoredRequest*(
            IncomingDiagConnection& connection, uint8_t const* request, uint16_t requestLength));

    MOCK_METHOD2(processAndReleaseRequest, void(AbstractDiagJob& job, StoredRequest& request));
};

} /* namespace uds */
