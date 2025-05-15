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
    MOCK_METHOD(void, canFrameSent, (CANFrame const&), (override));
    MOCK_METHOD(IFilter&, getFilter, (), (override));
};

} // namespace can
