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
#include "can/framemgmt/ICANFrameListener.h"
#include "can/framemgmt/IFilteredCANFrameSentListener.h"
#include "can/transceiver/ICANTransceiverStateListener.h"
#include "can/transceiver/ICanTransceiver.h"

#include <gmock/gmock.h>

namespace can
{
struct ICanTransceiverMock : public ICanTransceiver
{
    MOCK_METHOD(ErrorCode, init, ());
    MOCK_METHOD(void, shutdown, ());
    MOCK_METHOD(ErrorCode, open, (CANFrame const& frame));
    MOCK_METHOD(ErrorCode, open, ());
    MOCK_METHOD(ErrorCode, close, ());
    MOCK_METHOD(ErrorCode, mute, ());
    MOCK_METHOD(ErrorCode, unmute, ());
    MOCK_METHOD(State, getState, (), (const));

    MOCK_METHOD(ErrorCode, write, (CANFrame const& frame));
    MOCK_METHOD(ErrorCode, write, (CANFrame const& frame, ICANFrameSentListener& listener));

    MOCK_METHOD(uint32_t, getBaudrate, (), (const));
    MOCK_METHOD(uint16_t, getHwQueueTimeout, (), (const));
    MOCK_METHOD(uint8_t, getBusId, (), (const));

    MOCK_METHOD(void, addCANFrameListener, (ICANFrameListener&));
    MOCK_METHOD(void, addVIPCANFrameListener, (ICANFrameListener&));
    MOCK_METHOD(void, removeCANFrameListener, (ICANFrameListener&));

    MOCK_METHOD(void, addCANFrameSentListener, (IFilteredCANFrameSentListener&));
    MOCK_METHOD(void, removeCANFrameSentListener, (IFilteredCANFrameSentListener&));

    MOCK_METHOD(
        ICANTransceiverStateListener::CANTransceiverState, getCANTransceiverState, (), (const));
    MOCK_METHOD(void, setStateListener, (ICANTransceiverStateListener&));
    MOCK_METHOD(void, removeStateListener, ());
};

} // namespace can
