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
class TickHookMock : public ::estd::singleton<TickHookMock>
{
public:
    TickHookMock();

    static void handleTick();

    MOCK_METHOD0(doHandleTick, void());
};

} // namespace async
