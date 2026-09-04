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

#include "someip/IEventProvider.h"

#include <ip/IPAddress.h>

#include <gmock/gmock.h>

namespace someip
{
class EventProviderMock : public IEventProvider
{
public:
    MOCK_METHOD(
        bool,
        notifyInitialEvents,
        (service_id::type serviceId,
         instance_id::type instanceId,
         major_version::type majorVersion,
         eventgroup_id::type eventGroup,
         ::ip::IPAddress const& ipAddress,
         uint16_t port,
         uint8_t proto));
};

} // namespace someip
