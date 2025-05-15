// Copyright 2025 Accenture.

#pragma once

#include "safeMonitor/common.h"

#include <gmock/gmock.h>

namespace safeMonitor
{
template<
    typename Handler,
    typename Event,
    typename RegisterType,
    typename ScopedMutex = DefaultMutex,
    typename Context     = DefaultContext>
class RegisterMock
{
public:
    struct Entry
    {
        uintptr_t address;
        RegisterType valueMask;
        RegisterType expectedValue;
    };

    template<size_t N>
    RegisterMock(Handler&, Event const&, Entry const (&)[N])
    {}

    MOCK_METHOD(void, check, ());
    MOCK_CONST_METHOD0_T(getContext, Context&());
};
} // namespace safeMonitor
