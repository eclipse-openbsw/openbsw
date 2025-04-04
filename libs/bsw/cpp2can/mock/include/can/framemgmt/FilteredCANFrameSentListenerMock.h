// Copyright 2024 Accenture.

/**
 * Contains
 * \file
 * \ingroup
 */

#pragma once

#include "can/framemgmt/ICANFrameListener.h"
#include "can/framemgmt/IFilteredCANFrameSentListener.h"

#include <gmock/gmock.h>

namespace can
{
struct FilteredCANFrameSentListenerMock : public IFilteredCANFrameSentListener
{
    MOCK_METHOD1(canFrameSent, void(CANFrame const&));
    MOCK_METHOD0(getFilter, IFilter&());
};

} // namespace can
