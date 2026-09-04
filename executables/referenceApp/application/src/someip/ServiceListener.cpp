/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/ServiceListener.h"

#include "ip/IPEndpoint.h"
#include "someip/IServiceListener.h"
#include "someip/RpcChannel.h"
#include "someip/ServiceDescription.h"
#include "someip/ServiceInfo.h"
#include "someip/SomeIpConstants.h"
#include "someip/logger.h"

using ::util::logger::SOMEIP;

namespace
{

static constexpr uint16_t EXAMPLE_METHOD_ID = 1U;
static constexpr uint16_t EXAMPLE_EVENT_ID  = 0xFFFFU;
static constexpr uint16_t g_eventGroupIds[] = {0U};
static constexpr ::someip::EventDetail g_eventDetails[]
    = {{0xFFFFU, ::someip::SomeIpEventType::EVENT_TYPE_METHOD, g_eventGroupIds}};

static constexpr ::someip::MethodDetail g_methodDetails_[]
    = {{1U,
        ::someip::SomeIpMethodType::METHOD_REQUEST_RESPONSE,
        ::someip::SomeIpCallSemantic::SEMANTIC_REQUEST_RESPONSE}};

static constexpr ::etl::span<::someip::MethodDetail const> g_methodDetails(g_methodDetails_);

} // namespace

ServiceListener::ServiceListener(::someip::RpcChannel& rpcChannel) : _rpcChannel(rpcChannel) {}

void ServiceListener::serviceStatusChanged(
    ::someip::ServiceDescription const& service, ServiceStatus status)
{
    WARN_LOG(SOMEIP, "serviceStatusChanged");
    if (status == ::someip::IServiceListener::ServiceStatus::SERVICE_AVAILABLE)
    {
        WARN_LOG(SOMEIP, "open udp");
        _rpcChannel.openUdp(
            service.serviceId, ::ip::IPEndpoint(service.ipAddress, service.port), service.port);
    }
    else if (status == ::someip::IServiceListener::ServiceStatus::SERVICE_UNAVAILABLE)
    {
        _rpcChannel.close();
    }
}

void ServiceListener::updateEventgroupDescription(::someip::ServiceDescription&, ServiceStatus) {}

::someip::MethodDetail const* ServiceListener::getMethodDetail(uint16_t methodId) const
{
    switch (methodId)
    {
        case EXAMPLE_METHOD_ID:
        // [[fallthrough]]
        default:
        {
            return &g_methodDetails[0];
        }
    }
}

::someip::EventDetail const* ServiceListener::getEventDetail(uint16_t eventId) const
{
    switch (eventId)
    {
        case EXAMPLE_EVENT_ID:
        // [[fallthrough]]
        default:
        {
            return &g_eventDetails[0];
        }
    }
}
