// Copyright 2024 Accenture.

#pragma once

#include "can/canframes/ICANFrameSentListener.h"
#include "can/transceiver/AbstractCANTransceiver.h"

#include <gmock/gmock.h>

namespace can
{
struct AbstractCANTransceiverMock : public AbstractCANTransceiver
{
    AbstractCANTransceiverMock(uint8_t busId);

    virtual ~AbstractCANTransceiverMock() = default;

    MOCK_METHOD(ErrorCode, init, (), (override));

    MOCK_METHOD(void, shutdown, (), (override));

    MOCK_METHOD(ErrorCode, open, (CANFrame const& frame), (override));

    MOCK_METHOD(ErrorCode, open, (), (override));

    MOCK_METHOD(ErrorCode, close, (), (override));

    MOCK_METHOD(ErrorCode, mute, (), (override));

    MOCK_METHOD(ErrorCode, unmute, ());

    MOCK_METHOD(ErrorCode, write, (CANFrame const& frame), (override));

    MOCK_METHOD(
        ErrorCode, write, (CANFrame const& frame, ICANFrameSentListener& listener), (override));

    MOCK_METHOD(uint32_t, getBaudrate, (), (const, override));

    MOCK_METHOD(uint16_t, getHwQueueTimeout, (), (const, override));

    //        MOCK_CONST_METHOD0(getCANTransceiverState,
    //        can::ICANTransceiverStateListener::CANTransceiverState());

    void inject(CANFrame const& frame);

    void setTxTimestampForNextWrite(uint32_t timestamp);

    void setTransceiverState2(can::ICANTransceiverStateListener::CANTransceiverState state);

protected:
    State getStateImplementation() const;

    ErrorCode initImplementation();

    ErrorCode openImplementation();

    ErrorCode writeImplementation(CANFrame const& frame);

    ErrorCode writeImplementation2(CANFrame const& frame, ICANFrameSentListener& listener);

private:
    uint32_t _txTimestamp;
};

} // namespace can
