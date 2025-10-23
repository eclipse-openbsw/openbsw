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

    MOCK_METHOD(void, trigger, ());
    MOCK_METHOD(void, trigger, (Context const&));
    MOCK_METHOD(Context&, getContext, (), (const));
};
} // namespace safeMonitor
