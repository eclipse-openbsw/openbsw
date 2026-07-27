/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "middleware/core/MessagePayloadBuilder.h"
#include "middleware/core/ProxyBase.h"
#include "middleware/core/types.h"

#include <etl/expected.h>

#include <cstdint>

namespace middleware::rpc
{

/**
 * \brief Proxy-side representation of a fire-and-forget remote method.
 *
 * Sends a request message without tracking a response. No futures or callbacks are involved.
 */
class ProxyFireAndForgetMethod
{
public:
    /**
     * \brief Sends a fire-and-forget method call to a remote skeleton.
     *
     * Allocates the input payload into a message and sends it. No response is expected.
     *
     * \tparam InputType  The request payload type
     * \param proxy    The proxy through which the message is sent
     * \param input    The request payload
     * \param methodId The member ID identifying this method on the wire
     * \return INVALID_REQUEST_ID on success, or an HRESULT error
     */
    template<typename InputType>
    etl::expected<uint16_t, core::HRESULT>
    callMethod(core::ProxyBase const& proxy, InputType const& input, uint16_t const methodId);

    /**
     * \brief Sends a fire-and-forget method call with no input payload.
     *
     * Sends a header-only message. No response is expected.
     *
     * \param proxy    The proxy through which the message is sent
     * \param methodId The member ID identifying this method on the wire
     * \return INVALID_REQUEST_ID on success, or an HRESULT error
     */
    etl::expected<uint16_t, core::HRESULT>
    callMethod(core::ProxyBase const& proxy, uint16_t const methodId);

private:
    etl::expected<uint16_t, core::HRESULT> callMethodImpl(
        core::ProxyBase const& proxy, core::Message& msg, core::HRESULT allocationResult);
};

// ============================================================================
// Template definitions
// ============================================================================

template<typename InputType>
etl::expected<uint16_t, core::HRESULT> ProxyFireAndForgetMethod::callMethod(
    core::ProxyBase const& proxy, InputType const& input, uint16_t const methodId)
{
    if (!proxy.isInitialized())
    {
        return etl::unexpected<core::HRESULT>(etl::in_place, core::HRESULT::NotRegistered);
    }

    core::Message msg       = proxy.generateMessageHeader(methodId);
    core::HRESULT const ret = core::MessagePayloadBuilder::getInstance().allocate(input, msg);
    return callMethodImpl(proxy, msg, ret);
}

} // namespace middleware::rpc
