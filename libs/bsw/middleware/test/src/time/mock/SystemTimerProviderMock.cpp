/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "SystemTimerProviderMock.h"

#include "etl/error_handler.h"

#include "middleware/time/SystemTimerProvider.h"

namespace middleware::time
{

namespace
{
test::SystemTimerProviderMock* gSystemTimerProviderMockPtr = nullptr;
} // namespace

namespace test
{

void setSystemTimerProviderMock(SystemTimerProviderMock* const ptr)
{
    gSystemTimerProviderMockPtr = ptr;
}

void unsetSystemTimerProviderMock() { gSystemTimerProviderMockPtr = nullptr; }

} // namespace test

uint32_t getCurrentTimeInMs()
{
    if (gSystemTimerProviderMockPtr != nullptr)
    {
        return gSystemTimerProviderMockPtr->getCurrentTimeInMs();
    }

    ETL_ASSERT_FAIL("SystemTimerProviderMock is not set.");
}

uint32_t getCurrentTimeInUs()
{
    if (gSystemTimerProviderMockPtr != nullptr)
    {
        return gSystemTimerProviderMockPtr->getCurrentTimeInUs();
    }

    ETL_ASSERT_FAIL("SystemTimerProviderMock is not set.");
}

} // namespace middleware::time
