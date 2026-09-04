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

#include "someip/ISomeIpSerializable.h"
#include "someip/SomeIpMessage.h"

#include <ip/IPEndpoint.h>

namespace someip
{
class ServiceHandler;

class RequestContext
{
public:
    RequestContext();

    ::ip::IPEndpoint remoteIp;
    ISomeIpSerializable* pResponse;
    ISomeIpSerializable* pRequest;
    ServiceHandler* pService;
    void* callback;
    uint32_t requestId;
    service_id::type serviceId;
    uint16_t methodId;
    uint16_t localPort;
    uint8_t proto;
    uint8_t interfaceVersion;
    SomeIpMessage::MessageType requestMessageType;
    uint8_t operationResult;
};

/*
 * inline implementation
 */
inline RequestContext::RequestContext()
: remoteIp()
, pResponse(nullptr)
, pRequest(nullptr)
, pService(nullptr)
, callback(nullptr)
, requestId(0U)
, serviceId(0U)
, methodId(0U)
, localPort(0U)
, proto(0U)
, interfaceVersion(0U)
, requestMessageType(SomeIpMessage::MessageType::EXCEPTION)
, operationResult(0U)
{}

} // namespace someip
