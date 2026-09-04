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

#include <ip/IPAddress.h>

#include <etl/delegate.h>
#include <etl/optional.h>

namespace someip
{

class SomeIpMessage;

class IRpcReceiver
{
protected:
    IRpcReceiver() = default;

public:
    IRpcReceiver(IRpcReceiver const&)           = delete;
    IRpcReceiver& operator=(IRpcHandler const&) = delete;

    virtual ~IRpcReceiver() = default;

    /**
     * Pure virtual function that sets PriorityRpcHandler. If a PriorityRpcHandler
     * is set, it will be used before the RpcHandler.
     */
    virtual void setPriorityRpcHandler(IRpcHandler& priorityRpcHandler) = 0;

    /**
     * Pure virtual function that removes PriorityRpcHandler.
     */
    virtual void removePriorityRpcHandler() = 0;

    /**
     * Pure virtual function that requests multicast reception if available.
     */
    virtual bool requestMulticastReception(::ip::IPEndpoint const& multicastEndpoint) = 0;

    /**
     * Pure virtual function that cancels multicast reception if applicable.
     */
    virtual void cancelMulticastReception(::ip::IPEndpoint const& multicastEndpoint) = 0;
};

} // namespace someip
