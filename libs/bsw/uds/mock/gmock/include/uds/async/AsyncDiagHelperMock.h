// Copyright 2024 Accenture.

#pragma once

#include "uds/async/IAsyncDiagHelper.h"

#include <gmock/gmock.h>

namespace uds
{
class AsyncDiagHelperMock : public IAsyncDiagHelper
{
public:
    MOCK_METHOD(::async::ContextType, getDiagContext, (), (const, override));

    MOCK_METHOD(
        StoredRequest*,
        allocateRequest,
        (IncomingDiagConnection & connection, uint8_t const* request, uint16_t requestLength),
        (override));

    MOCK_METHOD(
        void,
        processAndReleaseRequest,
        (AbstractDiagJob & job, StoredRequest& request),
        (override));
};

} /* namespace uds */
