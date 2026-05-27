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

#include <bsp/can/canTransceiver/CanPhy.h>

#include <gmock/gmock.h>

namespace bios
{
class FlexCANDevice;

class CanPhyMock : public CanPhy
{
public:
    MOCK_METHOD(void, init, (uint32_t));

    bool setMode(Mode mode, uint32_t)
    {
        fMode = mode;
        return false;
    }

    Mode getMode(uint32_t) { return fMode; }

    MOCK_METHOD(ErrorCode, getPhyErrorStatus, (uint32_t id));

    // With the current CanFlex2Transceiver interface, we control the CanPhy
    // object but *not* the FlexCANDevice. We use the CanPhy object to get a
    // reference back to the FlexCANDevice constructed within
    // CanFlex2Transceiver.
    void setFlexCANDevice(::bios::FlexCANDevice* flexCANDevice) { fpFlexCANDevice = flexCANDevice; }

    ::bios::FlexCANDevice* getFlexCANDevice() { return fpFlexCANDevice; }

protected:
    Mode fMode;
    ::bios::FlexCANDevice* fpFlexCANDevice;
};

} // namespace bios
