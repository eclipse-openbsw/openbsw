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

#include "someip/ServiceDescription.h"

namespace someip
{
class IServiceTrackerListener
{
protected:
    IServiceTrackerListener& operator=(IServiceTrackerListener const&) = default;

public:
    enum class ServiceTrackerStatus : uint8_t
    {
        SERVICE_ADDED,
        SERVICE_REMOVED,
        SERVICE_CHANGED, // endpoint or proto
        SERVICE_RELIABLE
    };

    virtual ~IServiceTrackerListener() = default;

    /**
     * Pure virtual function that triggers updates due to changes of service tracker.
     */

    virtual void
    serviceTrackerChanged(ServiceDescription const& service, ServiceTrackerStatus status)
        = 0;
};

} // namespace someip
