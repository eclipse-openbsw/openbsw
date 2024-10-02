// Copyright 2024 Accenture.

/**
 * Contains
 * \file
 * \ingroup
 */

#ifndef GUARD_89879037_7F2D_437E_A5D3_6D66328887FD
#define GUARD_89879037_7F2D_437E_A5D3_6D66328887FD

#include "can/canframes/ICANFrameSentListener.h"

#include <gmock/gmock.h>

namespace can
{
struct CANFrameSentListenerMock : public ICANFrameSentListener
{
    MOCK_METHOD1(canFrameSent, void(CANFrame const&));
};

} // namespace can

#endif /* GUARD_89879037_7F2D_437E_A5D3_6D66328887FD */
