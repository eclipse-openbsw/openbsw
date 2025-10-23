// Copyright 2025 Accenture.

#pragma once

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

    MOCK_METHOD(void, hit, (Checkpoint const&));
    MOCK_METHOD(void, hit, (Checkpoint const&, Context const&));
    MOCK_METHOD(Context&, getContext, (), (const));
};
} // namespace safeMonitor
