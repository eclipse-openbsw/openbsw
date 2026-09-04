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
#include "someip/IEventReceiver.h"
#include "someip/SomeIpParser.h"

#include <gmock/gmock.h>

namespace someip
{
class EventReceiverMock : public IEventReceiver
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
    MOCK_METHOD(void, addEventListener, (IEventListener & listener));
    MOCK_METHOD(void, removeEventListener, (IEventListener & listener));
};

} // namespace someip
