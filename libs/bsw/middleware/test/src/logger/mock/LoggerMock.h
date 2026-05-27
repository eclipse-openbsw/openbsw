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

#include <vector>

#include <gmock/gmock.h>

#include "middleware/logger/Logger.h"

namespace middleware::logger::test::mock
{

class LoggerMock
{
public:
    LoggerMock();
    ~LoggerMock();

    MOCK_METHOD(void, log, (LogLevel const, char const* const, std::vector<uint32_t> const&));
    MOCK_METHOD(void, logBinary, (LogLevel const, etl::span<uint8_t const> const));
    MOCK_METHOD(uint32_t, getMessageId, (Error const));
};

} // namespace middleware::logger::test::mock
