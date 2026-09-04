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
#include "someip/RpcClosure.h"
#include "someip/SomeIpConstants.h"

#include <async/Types.h>

#include <etl/expected.h>
#include <etl/intrusive_links.h>
#include <cstdint>

namespace ip
{
class IPEndpoint;
} /* namespace ip */

namespace someip
{
class IRpcChannel : public ::etl::forward_link<0>
{
public:
    IRpcChannel() = default;

    IRpcChannel(IRpcChannel const&)            = delete;
    IRpcChannel& operator=(IRpcChannel const&) = delete;

    /**
     * Pure virtual function that returns service ID.
     */
    virtual service_id::type getServiceId() const = 0;

    /**
     * Pure virtual function that returns client ID.
     */
    virtual uint16_t getClientId() const = 0;

    /**
     * Pure virtual function that returns session ID.
     */
    virtual uint16_t getSessionId() const = 0;

    /**
     * Pure virtual function that sets session ID.
     */
    virtual void setSessionId(uint16_t sessionId) = 0;

    /**
     * Pure virtual function that returns remoteEndpoint.
     */
    virtual ::ip::IPEndpoint const& getRemoteIp() const = 0;

    /**
     * Pure virtual function that returns local port.
     */
    virtual ::etl::expected<uint16_t, PortError> getLocalPort() const = 0;

    /**
     * Pure virtual function that returns protocol version.
     */
    virtual uint8_t getProto() const = 0;

    /**
     * Pure virtual function that cancels the internal timeout.
     */
    virtual void cancelTimeout() = 0;

    /**
     * Pure virtual function that sets the internal timeout.
     */
    virtual void setTimeout(::async::ContextType const context, uint32_t timeout) = 0;

    /**
     * Pure virtual function that handles calling of specified method
     * within given timeout if possible. CallDoneClosure is used for storing
     * possible response data.
     */
    virtual ServiceResultCode callMethod(
        uint16_t methodId,
        ISomeIpSerializable const* pRequest,
        uint8_t interfaceVersion,
        ISomeIpSerializable* pResponse,
        CallDoneClosure& done,
        uint32_t timeout)
        = 0;

    /**
     * Pure virtual function that handles fire and forget method calling.
     */
    virtual ServiceResultCode callFireAndForgetMethod(
        uint16_t methodId, ISomeIpSerializable const* pRequest, uint8_t interfaceVersion)
        = 0;

    /**
     * Pure virtual function that returns response if available.
     */
    virtual ISomeIpSerializable* getResponse() = 0;

    /**
     * Pure virtual function that can be used for processing of response result.
     */
    virtual void responseReceived(ServiceResultCode result) = 0;

protected:
    ~IRpcChannel() = default;
};

} // namespace someip
