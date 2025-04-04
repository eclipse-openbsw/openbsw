// Copyright 2024 Accenture.

/**
 * Contains
 * \file
 * \ingroup
 */

#pragma once

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
