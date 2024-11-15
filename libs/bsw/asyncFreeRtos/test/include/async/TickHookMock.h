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
class TickHookMock : public ::etl::singleton_base<TickHookMock>
{
public:
    TickHookMock();

    static void handleTick();

    MOCK_METHOD0(doHandleTick, void());
};

} // namespace async
