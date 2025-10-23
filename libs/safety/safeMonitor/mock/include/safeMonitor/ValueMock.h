// Copyright 2025 Accenture.

#pragma once

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

    MOCK_METHOD(void, check, (ValueType const&));
    MOCK_METHOD(void, check, (ValueType const&, Context const&));
    MOCK_METHOD(Context&, getContext, (), (const));
};
} // namespace safeMonitor
