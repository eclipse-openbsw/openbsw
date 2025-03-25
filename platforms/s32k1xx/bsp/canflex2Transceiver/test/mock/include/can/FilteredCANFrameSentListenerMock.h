// Copyright 2024 Accenture.

#pragma once

#include <can/framemgmt/IFilteredCANFrameSentListener.h>

#include <platform/estdint.h>

#include <gmock/gmock.h>

namespace can
{
class FilteredCANFrameSentListenerMock : public ::can::IFilteredCANFrameSentListener
{
public:
    MOCK_METHOD1(canFrameSent, void(CANFrame const&));
    MOCK_METHOD0(getFilter, IFilter&());
};

} // namespace can
