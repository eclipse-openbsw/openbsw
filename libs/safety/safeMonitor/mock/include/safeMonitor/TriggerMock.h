// Copyright 2025 Accenture.

#ifndef GUARD_1B1ACB31_6FAA_4BA5_AB42_822D4AEDD75B
#define GUARD_1B1ACB31_6FAA_4BA5_AB42_822D4AEDD75B

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

#endif // GUARD_1B1ACB31_6FAA_4BA5_AB42_822D4AEDD75B
