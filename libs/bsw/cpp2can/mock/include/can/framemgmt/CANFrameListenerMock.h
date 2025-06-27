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
    MOCK_METHOD(void, frameReceived, (CANFrame const&), (override));
    MOCK_METHOD(IFilter&, getFilter, (), (override));
};

} // namespace can
