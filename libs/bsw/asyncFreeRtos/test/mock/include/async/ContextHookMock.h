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
class ContextHookMock : public ::etl::singleton_base<ContextHookMock>
{
public:
    ContextHookMock();

    static void enterTask(size_t taskIdx);
    static void leaveTask(size_t taskIdx);
    static void enterIsrGroup(size_t isrGroupIdx);
    static void leaveIsrGroup(size_t isrGroupIdx);

    MOCK_METHOD(void, doEnterTask, (size_t));
    MOCK_METHOD(void, doLeaveTask, (size_t));
    MOCK_METHOD(void, doEnterIsrGroup, (size_t));
    MOCK_METHOD(void, doLeaveIsrGroup, (size_t));
};

} // namespace async
