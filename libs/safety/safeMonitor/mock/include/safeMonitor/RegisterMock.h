// Copyright 2025 Accenture.

#ifndef GUARD_98AE4C33_C22C_418E_811D_8FB82C387F2D
#define GUARD_98AE4C33_C22C_418E_811D_8FB82C387F2D

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

    MOCK_METHOD0(check, void());
    MOCK_CONST_METHOD0_T(getContext, Context&());
};
} // namespace safeMonitor

#endif // GUARD_98AE4C33_C22C_418E_811D_8FB82C387F2D
