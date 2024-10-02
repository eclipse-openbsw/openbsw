// Copyright 2024 Accenture.

#ifndef GUARD_52494E61_3165_40EE_BFE4_F03401EEFCD4
#define GUARD_52494E61_3165_40EE_BFE4_F03401EEFCD4

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

#endif /* GUARD_52494E61_3165_40EE_BFE4_F03401EEFCD4 */
