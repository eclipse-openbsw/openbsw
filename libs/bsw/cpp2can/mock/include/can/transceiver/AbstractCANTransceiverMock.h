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

    MOCK_METHOD0(init, ErrorCode());

    MOCK_METHOD0(shutdown, void());

    MOCK_METHOD1(open, ErrorCode(CANFrame const& frame));

    MOCK_METHOD0(open, ErrorCode());

    MOCK_METHOD0(close, ErrorCode());

    MOCK_METHOD0(mute, ErrorCode());

    MOCK_METHOD0(unmute, ErrorCode());

    MOCK_METHOD1(write, ErrorCode(CANFrame const& frame));

    MOCK_METHOD2(write, ErrorCode(CANFrame const& frame, ICANFrameSentListener& listener));

    MOCK_CONST_METHOD0(getBaudrate, uint32_t());

    MOCK_CONST_METHOD0(getHwQueueTimeout, uint16_t());

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

