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
    MOCK_METHOD(void, canFrameSent, (CANFrame const&));
    MOCK_METHOD(IFilter&, getFilter, ());
};

} // namespace can
