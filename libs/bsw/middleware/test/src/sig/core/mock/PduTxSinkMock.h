/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <gmock/gmock.h>

#include "middleware/sig/core/IPduTxSink.h"

namespace middleware::sig::core::test
{

class PduTxSinkMock : public IPduTxSink
{
public:
    MOCK_METHOD(void, submit, (uint32_t, ::etl::span<uint8_t const>), (override));
};

} // namespace middleware::sig::core::test
