// Copyright 2025 Accenture.

#ifndef GUARD_E0F96198_0A35_4F6A_9093_AFBBB46C60B4
#define GUARD_E0F96198_0A35_4F6A_9093_AFBBB46C60B4

#include "safeMonitor/common.h"

#include <gmock/gmock.h>

namespace safeMonitor
{
template<
    typename Handler,
    typename Event,
    typename ValueType,
    typename ScopedMutex = DefaultMutex,
    typename Context     = DefaultContext>
class ValueMock
{
public:
    ValueMock(Handler&, Event const&, ValueType const&) {}

    MOCK_METHOD1_T(check, void(ValueType const&));
    MOCK_METHOD2_T(check, void(ValueType const&, Context const&));
    MOCK_CONST_METHOD0_T(getContext, Context&());
};
} // namespace safeMonitor

#endif // GUARD_E0F96198_0A35_4F6A_9093_AFBBB46C60B4
