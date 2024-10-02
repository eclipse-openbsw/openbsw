// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup async
 */
#ifndef GUARD_D63B58A1_669D_4AE1_8FD5_455D98523C5D
#define GUARD_D63B58A1_669D_4AE1_8FD5_455D98523C5D

#include "estd/singleton.h"

#include <gmock/gmock.h>

namespace async
{
class TickHookMock : public ::estd::singleton<TickHookMock>
{
public:
    TickHookMock();

    static void handleTick();

    MOCK_METHOD0(doHandleTick, void());
};

} // namespace async

#endif // GUARD_D63B58A1_669D_4AE1_8FD5_455D98523C5D
