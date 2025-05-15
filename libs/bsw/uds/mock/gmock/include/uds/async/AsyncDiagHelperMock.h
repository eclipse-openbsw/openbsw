// Copyright 2024 Accenture.

#pragma once

#include "uds/async/IAsyncDiagHelper.h"

#include <gmock/gmock.h>

namespace uds
{
class AsyncDiagHelperMock : public IAsyncDiagHelper
{
public:
    MOCK_METHOD(::async::ContextType, getDiagContext, (), (const));

    MOCK_METHOD(
        StoredRequest*,
        allocateRequest,
        (IncomingDiagConnection & connection, uint8_t const* request, uint16_t requestLength));

    MOCK_METHOD(void, processAndReleaseRequest, (AbstractDiagJob & job, StoredRequest& request));
};

} /* namespace uds */
