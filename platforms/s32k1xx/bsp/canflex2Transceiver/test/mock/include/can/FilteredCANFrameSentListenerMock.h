/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
