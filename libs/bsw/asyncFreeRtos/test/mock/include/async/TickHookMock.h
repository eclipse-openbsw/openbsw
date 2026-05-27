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
 * \file
 * \ingroup async
 */
#pragma once

#include <etl/singleton_base.h>

#include <gmock/gmock.h>

namespace async
{
class TickHookMock : public ::etl::singleton_base<TickHookMock>
{
public:
    TickHookMock();

    static void handleTick();

    MOCK_METHOD(void, doHandleTick, ());
};

} // namespace async
