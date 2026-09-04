/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/SubscriptionEndpoint.h"

#include "someip/ServiceDescription.h"

namespace someip
{
SubscriptionEndpoint::SubscriptionEndpoint() : IPEndpoint(), ttl(ttl::INVALID), index(DEFAULT_INDEX)
{}

SubscriptionEndpoint::SubscriptionEndpoint(::ip::IPAddress const& ipAddr, uint16_t const port)
: IPEndpoint(ipAddr, port), ttl(ttl::INVALID), index(DEFAULT_INDEX)
{}

SubscriptionEndpoint::SubscriptionEndpoint(SubscriptionEndpoint const& other)
: ::etl::forward_link<0>(::etl::forward_link<0>())
, IPEndpoint(other)
, ttl(other.ttl)
, index(other.index)
{}

bool SubscriptionEndpoint::isValid() const { return isSet() && (ttl != ttl::INVALID); }

void SubscriptionEndpoint::clear()
{
    IPEndpoint::clear();
    ttl   = ttl::INVALID;
    index = DEFAULT_INDEX;
}

SubscriptionEndpoint& SubscriptionEndpoint::operator=(SubscriptionEndpoint const& other)
{
    if (this != &other)
    {
        setAddress(other.getAddress());
        setPort(other.getPort());
        ttl   = other.ttl;
        index = other.index;
    }
    return *this;
}

} // namespace someip
