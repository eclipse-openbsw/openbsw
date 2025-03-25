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
class Watchdog
{
public:
    typedef uint32_t Timeout;

    constexpr Watchdog(Handler const&, Event const&, Timeout const) {}

    MOCK_METHOD0(kick, void());
    MOCK_METHOD1_T(kick, void(Context const&));
    MOCK_METHOD0(service, void());
    MOCK_CONST_METHOD0_T(getContext, Context&());
};
} // namespace safeMonitor
