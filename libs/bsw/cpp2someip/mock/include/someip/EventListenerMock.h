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

#include "someip/IEventListener.h"
#include "someip/SomeIpParser.h"

#include <gmock/gmock.h>

namespace someip
{
class EventListenerMock : public IEventListener
{
public:
    MOCK_METHOD(
        void,
        eventReceived,
        (service_id::type serviceId,
         uint16_t eventId,
         instance_id::type instanceId,
         major_version::type majorVersion,
         SomeIpParser& parser));
};

} // namespace someip
