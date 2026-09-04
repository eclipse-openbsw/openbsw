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

#include "someip/SomeIpConstants.h"

#include <ip/IPEndpoint.h>

#include <etl/intrusive_forward_list.h>
#include <etl/intrusive_links.h>

namespace someip
{
struct SubscriptionEndpoint
: ::etl::forward_link<0>
, ::ip::IPEndpoint
{
    // The default value for index, which is set when constructed or cleared.
    static constexpr uint8_t DEFAULT_INDEX = 0xFFU;

    SubscriptionEndpoint();
    SubscriptionEndpoint(::ip::IPAddress const& ipAddr, uint16_t port);
    SubscriptionEndpoint(SubscriptionEndpoint const& other);

    bool isValid() const;
    void clear();

    SubscriptionEndpoint& operator=(SubscriptionEndpoint const& other);

    ::someip::ttl::type ttl;

    // This member can be used for caching in other applications. This might be useful if
    // something in your application is done e.g. with every event being sent and is actually
    // bound to this subscription.
    // The access is not synchronized, so it can only be used by one application.
    // This uint8_t is placed here (violating our coding guidelines to sort members be size in
    // descending order) because the base class ::ip::IPEndpoint ends with an uint16_t and uint8_t.
    uint8_t index;
};

using SubscriptionEndpointList
    = ::etl::intrusive_forward_list<SubscriptionEndpoint, ::etl::forward_link<0>>;

} // namespace someip
