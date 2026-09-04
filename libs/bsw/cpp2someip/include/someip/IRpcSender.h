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

#include "someip/RpcClosure.h"
#include "someip/SomeIpConstants.h"

#include <cstdint>

namespace someip
{
class ISomeIpSerializable;
class IRpcChannel;

class IRpcSender
{
protected:
    IRpcSender() = default;

public:
    IRpcSender(IRpcSender const&)            = delete;
    IRpcSender& operator=(IRpcSender const&) = delete;

    /**
     * Send request to remote ECU
     *
     * \param pRequest Payload of the request
     * \param serviceId ServiceID of the request
     * \param MethodId MethodID of the request
     * \param InterfaceVersion InterfaceVersion of the request
     * \param isResponseExpected request-response (true) or fire&forget (false)
     * \param channel IRpcChannel storing Endpoint information
     * \param timeout time in ms until request is expired
     *
     * \return Result of sending attempt in form of a ServiceResultCode
     */
    virtual ServiceResultCode sendRequest(
        ISomeIpSerializable const* pRequest,
        service_id::type serviceId,
        uint16_t methodId,
        uint8_t interfaceVersion,
        bool isResponseExpected,
        IRpcChannel& channel,
        uint32_t timeout)
        = 0;

    /**
     * Process expired request (no response sent in time).
     *
     * \param channel affected channel
     *
     */
    virtual void requestExpired(IRpcChannel& channel) = 0;
};

} // namespace someip
