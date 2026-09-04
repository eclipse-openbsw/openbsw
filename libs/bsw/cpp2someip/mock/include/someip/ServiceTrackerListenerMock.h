/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "someip/IServiceTrackerListener.h"

#include <gmock/gmock.h>

namespace someip
{
class ServiceTrackerListenerMock : public IServiceTrackerListener
{
public:
    MOCK_METHOD(
        void,
        serviceTrackerChanged,
        (ServiceDescription const& service, ServiceTrackerStatus status));
};

} // namespace someip
