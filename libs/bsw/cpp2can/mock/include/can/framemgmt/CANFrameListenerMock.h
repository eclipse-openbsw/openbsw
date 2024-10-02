// Copyright 2024 Accenture.

/**
 * Contains
 * \file
 * \ingroup
 */

#ifndef GUARD_56BEB4E3_3717_422C_9CE3_8934FA806DDF
#define GUARD_56BEB4E3_3717_422C_9CE3_8934FA806DDF

#include "can/framemgmt/ICANFrameListener.h"

#include <gmock/gmock.h>

namespace can
{
struct CANFrameListenerMock : public ICANFrameListener
{
    MOCK_METHOD1(frameReceived, void(CANFrame const&));
    MOCK_METHOD0(getFilter, IFilter&());
};

} // namespace can

#endif /* GUARD_56BEB4E3_3717_422C_9CE3_8934FA806DDF */
