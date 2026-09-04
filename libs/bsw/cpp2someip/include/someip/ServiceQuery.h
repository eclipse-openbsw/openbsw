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

#include "someip/SdConfig.h"
#include "someip/ServiceDescription.h"

#include <ip/IPEndpoint.h>

namespace someip
{
class IServiceListener;

struct ServiceQuery
{
    enum class ServiceQueryState : uint8_t
    {
        QUERY_IDLE_PHASE,
        QUERY_INITIAL_WAIT_PHASE,
        QUERY_REPETITION_PHASE,
        QUERY_MAIN_PHASE
    };

    enum class SubscriptionState : uint8_t
    {
        STATE_UNSUBSCRIBED,
        STATE_WAITING_FOR_ACK,
        STATE_ACK_RECEIVED
    };

    ::ip::IPEndpoint multicastAddress;
    uint64_t timestamp;
    ServiceQueryState state;
    uint32_t repetitionCount;
    SubscriptionState subscriptionState;
    ServiceDescription description;
    IServiceListener* listener;
    ::ip::IPAddress serviceDiscoveryAddress;
    SdConfig sdConfig;
};

} // namespace someip
