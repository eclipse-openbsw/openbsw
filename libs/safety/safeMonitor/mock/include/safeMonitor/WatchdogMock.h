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

    MOCK_METHOD(void, kick, ());
    MOCK_METHOD(void, kick, (Context const&));
    MOCK_METHOD(void, service, ());
    MOCK_METHOD(Context&, getContext, (), (const));
};
} // namespace safeMonitor
