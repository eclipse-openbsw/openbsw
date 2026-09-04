/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/ServiceQuery.h"

#include "someip/init.h"

namespace someip
{
template<>
ServiceQuery make<ServiceQuery>()
{
    // clang-format off
    return {
         /* multicastAddress */ ::ip::IPEndpoint(),
                /* timestamp */ 0U,
                    /* state */ ServiceQuery::ServiceQueryState::QUERY_IDLE_PHASE,
          /* repetitionCount */ 0U,
        /* subscriptionState */ ServiceQuery::SubscriptionState::STATE_UNSUBSCRIBED,
              /* description */ make<ServiceDescription>(),
                 /* listener */ nullptr,
                 #ifdef PLATFORM_SUPPORT_IPV6
  /* serviceDiscoveryAddress */ ::ip::make_ip6(0U), 
                 #else
  /* serviceDiscoveryAddress */ ::ip::make_ip4(0U), 
                 #endif
                 /* sdConfig */ {}
    };
    // clang-format on
}

template<>
void init<ServiceQuery>(ServiceQuery& ref)
{
    ref.multicastAddress  = ::ip::IPEndpoint();
    ref.timestamp         = 0U;
    ref.state             = ServiceQuery::ServiceQueryState::QUERY_INITIAL_WAIT_PHASE;
    ref.repetitionCount   = 0U;
    ref.subscriptionState = ServiceQuery::SubscriptionState::STATE_UNSUBSCRIBED;
}

} // namespace someip
