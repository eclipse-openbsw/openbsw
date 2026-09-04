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

#include "someip/ISomeIpStack.h"

#include <gmock/gmock.h>

namespace someip
{
class SomeIpStackMock : public ISomeIpStack
{
public:
    /* LIFECYCLE */

    MOCK_METHOD(bool, initSdPort, (uint16_t port));
    MOCK_METHOD(bool, initUdpPort, (uint16_t port));
    MOCK_METHOD(bool, initTcpPort, (uint16_t port));

    /* CLIENT */

    MOCK_METHOD(bool, registerServiceQuery, (ServiceQuery & query));
    MOCK_METHOD(void, unregisterServiceQuery, (ServiceQuery & query));

    MOCK_METHOD(void, addEventListener, (IEventListener & listener));
    MOCK_METHOD(void, removeEventListener, (IEventListener & listener));

    /* SERVER */

    MOCK_METHOD(bool, registerProvidedService, (ProvidedService & service));
    MOCK_METHOD(void, unregisterProvidedService, (ProvidedService & service));
};

} // namespace someip
