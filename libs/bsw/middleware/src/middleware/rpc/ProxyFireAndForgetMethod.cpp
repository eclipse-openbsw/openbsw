/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "middleware/rpc/ProxyFireAndForgetMethod.h"

#include "middleware/core/MessagePayloadBuilder.h"

namespace middleware::rpc
{

etl::expected<uint16_t, core::HRESULT>
ProxyFireAndForgetMethod::callMethod(core::ProxyBase const& proxy, uint16_t const methodId)
{
    if (!proxy.isInitialized())
    {
        return etl::unexpected<core::HRESULT>(etl::in_place, core::HRESULT::NotRegistered);
    }

    core::Message msg = proxy.generateMessageHeader(methodId);
    return callMethodImpl(proxy, msg, core::HRESULT::Ok);
}

etl::expected<uint16_t, core::HRESULT> ProxyFireAndForgetMethod::callMethodImpl(
    core::ProxyBase const& proxy, core::Message& msg, core::HRESULT allocationResult)
{
    if (allocationResult != core::HRESULT::Ok)
    {
        return etl::unexpected<core::HRESULT>(etl::in_place, allocationResult);
    }

    core::HRESULT sendResult = proxy.sendMessage(msg);
    if (sendResult != core::HRESULT::Ok)
    {
        core::MessagePayloadBuilder::deallocate(msg);
        return etl::unexpected<core::HRESULT>(etl::in_place, sendResult);
    }

    return core::INVALID_REQUEST_ID; // Fire-and-forget methods do not have a request ID
}

} // namespace middleware::rpc
