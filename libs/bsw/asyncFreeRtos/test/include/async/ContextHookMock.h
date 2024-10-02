// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup async
 */
#ifndef GUARD_94539508_7E46_4553_9CEE_CEA6E7B176ED
#define GUARD_94539508_7E46_4553_9CEE_CEA6E7B176ED

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

#endif // GUARD_94539508_7E46_4553_9CEE_CEA6E7B176ED
