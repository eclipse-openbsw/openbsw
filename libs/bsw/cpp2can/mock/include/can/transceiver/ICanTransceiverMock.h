// Copyright 2024 Accenture.

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
    MOCK_METHOD(ErrorCode, init, (), (override));
    MOCK_METHOD(void, shutdown, (), (override));
    MOCK_METHOD(ErrorCode, open, (CANFrame const& frame), (override));
    MOCK_METHOD(ErrorCode, open, (), (override));
    MOCK_METHOD(ErrorCode, close, (), (override));
    MOCK_METHOD(ErrorCode, mute, (), (override));
    MOCK_METHOD(ErrorCode, unmute, (), (override));
    MOCK_METHOD(State, getState, (), (const, override));

    MOCK_METHOD(ErrorCode, write, (CANFrame const& frame), (override));
    MOCK_METHOD(
        ErrorCode, write, (CANFrame const& frame, ICANFrameSentListener& listener), (override));

    MOCK_METHOD(uint32_t, getBaudrate, (), (const, override));
    MOCK_METHOD(uint16_t, getHwQueueTimeout, (), (const, override));
    MOCK_METHOD(uint8_t, getBusId, (), (const, override));

    MOCK_METHOD(void, addCANFrameListener, (ICANFrameListener&), (override));
    MOCK_METHOD(void, addVIPCANFrameListener, (ICANFrameListener&), (override));
    MOCK_METHOD(void, removeCANFrameListener, (ICANFrameListener&), (override));

    MOCK_METHOD(void, addCANFrameSentListener, (IFilteredCANFrameSentListener&), (override));
    MOCK_METHOD(void, removeCANFrameSentListener, (IFilteredCANFrameSentListener&), (override));

    MOCK_METHOD(
        ICANTransceiverStateListener::CANTransceiverState,
        getCANTransceiverState,
        (),
        (const, override));
    MOCK_METHOD(void, setStateListener, (ICANTransceiverStateListener&), (override));
    MOCK_METHOD(void, removeStateListener, (), (override));
    MOCK_METHOD(void, setCANFrameSentListener, (IFilteredCANFrameSentListener*), (override));
};

} // namespace can
