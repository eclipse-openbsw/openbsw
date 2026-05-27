/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

/**
 * \ingroup async
 */
#pragma once

#include "async/Async.h"

#include <etl/singleton_base.h>

#include <gmock/gmock.h>

namespace async
{
class AsyncMock : public ::etl::singleton_base<AsyncMock>
{
public:
    AsyncMock() : ::etl::singleton_base<AsyncMock>(*this) {}

    MOCK_METHOD(void, execute, (ContextType contextType, RunnableType& runnableType));
    MOCK_METHOD(
        void,
        schedule,
        (ContextType contextType,
         RunnableType& runnableType,
         TimeoutType& timeoutType,
         uint32_t delay,
         TimeUnitType unit));
    MOCK_METHOD(
        void,
        scheduleAtFixedRate,
        (ContextType context,
         RunnableType& runnableType,
         TimeoutType& timeout,
         uint32_t period,
         TimeUnitType unit));
};

} // namespace async
