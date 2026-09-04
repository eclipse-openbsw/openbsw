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

#include "someip/IRpcHandler.h"
#include "someip/IRpcReceiver.h"

#include <ip/IPEndpoint.h>

#include <gmock/gmock.h>

namespace someip
{
class RpcReceiverMock : public IRpcReceiver
{
public:
    MOCK_METHOD(void, setPriorityRpcHandler, (IRpcHandler & priorityRpcHandler));
    MOCK_METHOD(void, removePriorityRpcHandler, ());

    MOCK_METHOD(bool, requestMulticastReception, (::ip::IPEndpoint const& multicastIp));
    MOCK_METHOD(void, cancelMulticastReception, (::ip::IPEndpoint const& multicastIp));
};

} // namespace someip
