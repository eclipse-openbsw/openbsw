// Copyright 2024 Accenture.

/**
 * Contains
 * \file
 * \ingroup
 */

#pragma once

#include "can/canframes/ICANFrameSentListener.h"

#include <gmock/gmock.h>

namespace can
{
struct CANFrameSentListenerMock : public ICANFrameSentListener
{
    MOCK_METHOD1(canFrameSent, void(CANFrame const&));
};

} // namespace can

