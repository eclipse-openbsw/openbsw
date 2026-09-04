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

#include "someip/ServiceHandler.h"

#include <etl/pool.h>
#include <gmock/gmock.h>

namespace someip
{
template<size_t NUM_CALLBACKS>
class ServiceHandlerMock : public ServiceHandler
{
public:
    ServiceHandlerMock();

    MOCK_METHOD(MethodDetail const*, getMethodDetail, (uint16_t methodId), (const));

    MOCK_METHOD(
        void,
        callMethod,
        (uint16_t methodId,
         ISomeIpSerializable const* pRequest,
         ISomeIpSerializable* pResponse,
         CallDoneClosure& done));

    MOCK_METHOD(void, onEventGroupSubscriptionStateChanged, (uint16_t, bool));
    MOCK_METHOD(ISomeIpSerializable*, getResponse, (uint16_t methodId));
    MOCK_METHOD(ISomeIpSerializable*, getRequest, (uint16_t methodId));

    MOCK_METHOD(
        bool,
        notifyInitialEvents,
        (service_id::type serviceId,
         instance_id::type instanceId,
         major_version::type majorVersion,
         eventgroup_id::type eventGroup,
         ::ip::IPAddress const& ipAddress,
         uint16_t port,
         uint8_t proto));

private:
    ::etl::pool<ServiceHandler::RpcCallback, NUM_CALLBACKS> _callbacks;
};

/*
 * inline implementation
 */
template<size_t NUM_CALLBACKS>
inline ServiceHandlerMock<NUM_CALLBACKS>::ServiceHandlerMock() : ServiceHandler(_callbacks)
{}

} // namespace someip
