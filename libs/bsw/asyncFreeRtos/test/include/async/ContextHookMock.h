// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup async
 */
#pragma once

#include "estd/singleton.h"

#include <gmock/gmock.h>

namespace async
{
class ContextHookMock : public ::estd::singleton<ContextHookMock>
{
public:
    ContextHookMock();

    static void enterTask(size_t taskIdx);
    static void leaveTask(size_t taskIdx);
    static void enterIsrGroup(size_t isrGroupIdx);
    static void leaveIsrGroup(size_t isrGroupIdx);

    MOCK_METHOD1(doEnterTask, void(size_t));
    MOCK_METHOD1(doLeaveTask, void(size_t));
    MOCK_METHOD1(doEnterIsrGroup, void(size_t));
    MOCK_METHOD1(doLeaveIsrGroup, void(size_t));
};

} // namespace async

