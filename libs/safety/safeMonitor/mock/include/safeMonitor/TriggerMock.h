// Copyright 2025 Accenture.

#pragma once

#include "safeMonitor/common.h"

#include <gmock/gmock.h>

namespace safeMonitor
{
template<
    typename Handler,
    typename Event,
    typename ScopedMutex = DefaultMutex,
    typename Context     = DefaultContext>
class TriggerMock
{
public:
    TriggerMock(Handler&, Event const&) {}

    MOCK_METHOD0(trigger, void());
    MOCK_METHOD1_T(trigger, void(Context const&));
    MOCK_CONST_METHOD0_T(getContext, Context&());
};
} // namespace safeMonitor
