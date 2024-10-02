// Copyright 2024 Accenture.

#ifndef GUARD_33F444F5_F396_4FCE_B674_2A81FA99DFE1
#define GUARD_33F444F5_F396_4FCE_B674_2A81FA99DFE1

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
    MOCK_METHOD0(init, ErrorCode());
    MOCK_METHOD0(shutdown, void());
    MOCK_METHOD1(open, ErrorCode(CANFrame const& frame));
    MOCK_METHOD0(open, ErrorCode());
    MOCK_METHOD0(close, ErrorCode());
    MOCK_METHOD0(mute, ErrorCode());
    MOCK_METHOD0(unmute, ErrorCode());
    MOCK_CONST_METHOD0(getState, State());

    MOCK_METHOD1(write, ErrorCode(CANFrame const& frame));
    MOCK_METHOD2(write, ErrorCode(CANFrame const& frame, ICANFrameSentListener& listener));

    MOCK_CONST_METHOD0(getBaudrate, uint32_t());
    MOCK_CONST_METHOD0(getHwQueueTimeout, uint16_t());
    MOCK_CONST_METHOD0(getBusId, uint8_t());

    MOCK_METHOD1(addCANFrameListener, void(ICANFrameListener&));
    MOCK_METHOD1(addVIPCANFrameListener, void(ICANFrameListener&));
    MOCK_METHOD1(removeCANFrameListener, void(ICANFrameListener&));

    MOCK_METHOD1(addCANFrameSentListener, void(IFilteredCANFrameSentListener&));
    MOCK_METHOD1(removeCANFrameSentListener, void(IFilteredCANFrameSentListener&));

    MOCK_CONST_METHOD0(getCANTransceiverState, ICANTransceiverStateListener::CANTransceiverState());
    MOCK_METHOD1(setStateListener, void(ICANTransceiverStateListener&));
    MOCK_METHOD0(removeStateListener, void());
    MOCK_METHOD1(setCANFrameSentListener, void(IFilteredCANFrameSentListener*));
};

} // namespace can

#endif /* GUARD_33F444F5_F396_4FCE_B674_2A81FA99DFE1 */
