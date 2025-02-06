// Copyright 2025 Accenture.

#ifndef GUARD_31A06FA7_27C6_4EBA_A312_272438CC7F86
#define GUARD_31A06FA7_27C6_4EBA_A312_272438CC7F86

#include "safeMonitor/common.h"

#include <gmock/gmock.h>

namespace safeMonitor
{
template<
    typename Handler,
    typename Event,
    typename Checkpoint,
    typename ScopedMutex = DefaultMutex,
    typename Context     = DefaultContext>
class SequenceMock
{
public:
    SequenceMock(Handler&, Event const&, Checkpoint const&, Checkpoint const&) {}

    MOCK_METHOD1_T(hit, void(Checkpoint const&));
    MOCK_METHOD2_T(hit, void(Checkpoint const&, Context const&));
    MOCK_CONST_METHOD0_T(getContext, Context&());
};
} // namespace safeMonitor

#endif // GUARD_31A06FA7_27C6_4EBA_A312_272438CC7F86
