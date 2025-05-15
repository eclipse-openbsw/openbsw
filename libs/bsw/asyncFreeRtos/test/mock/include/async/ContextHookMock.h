// Copyright 2024 Accenture.

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
