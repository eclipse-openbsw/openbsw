/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
