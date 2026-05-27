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
