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

#include "someip/IServiceListener.h"

#include <gmock/gmock.h>

namespace someip
{
class ServiceListenerMock : public IServiceListener
{
public:
    MOCK_METHOD(
        void, serviceStatusChanged, (ServiceDescription const& service, ServiceStatus status));
    MOCK_METHOD(
        void, updateEventgroupDescription, (ServiceDescription & service, ServiceStatus status));
    MOCK_METHOD(MethodDetail const*, getMethodDetail, (uint16_t methodId), (const));
    MOCK_METHOD(EventDetail const*, getEventDetail, (uint16_t eventId), (const));
};

} // namespace someip
