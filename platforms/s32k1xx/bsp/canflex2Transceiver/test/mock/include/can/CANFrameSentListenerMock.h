// Copyright 2024 Accenture.

#pragma once

#include "can/canframes/ICANFrameSentListener.h"

#include <platform/estdint.h>

#include <gmock/gmock.h>

namespace can
{
class CANFrameSentListenerMock : public ::can::ICANFrameSentListener
{
public:
    CANFrameSentListenerMock() : numCalls(0) {}

    void canFrameSent(CANFrame const&) { ++numCalls; }

    uint32_t getNumCalls() { return numCalls; }

    uint32_t numCalls;
};

} // namespace can

