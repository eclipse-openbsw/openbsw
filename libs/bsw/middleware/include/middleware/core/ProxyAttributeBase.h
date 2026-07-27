/********************************************************************************
 * Copyright (c) 2025, 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "middleware/core/ProxyBase.h"
#include "middleware/core/types.h"
#include "middleware/rpc/ProxyMethod.h"

#include <etl/delegate.h>
#include <etl/expected.h>

#include <cstdint>

namespace middleware::core
{

/**
 * Base class for proxy attribute getter functionality.
 * Provides async getter dispatch with future-based request tracking.
 *
 * \tparam REQUEST_LIMIT Maximum concurrent getter requests
 * \tparam DispatcherTraits Traits describing the getter argument type, member ID, etc.
 */
template<typename RpcTraits = void>
class ProxyAttributeBase
{
    using GetterMethodTraits = RpcTraits;
    using GetterMethod       = rpc::ProxyMethod<RpcTraits>;

public:
    using AttributeType  = typename GetterMethodTraits::OutputType;
    using GetterResult   = typename GetterMethod::Result;
    using GetterCallback = typename GetterMethod::Callback;

    explicit ProxyAttributeBase(ProxyBase& proxy) : _proxy(&proxy) {}

    /**
     * Initiates an asynchronous attribute retrieval request.
     *
     * \param methodId The member ID identifying the getter on the wire
     * \param attrCb   Callback invoked with either the attribute value or an error state
     * \return RequestId on success, or HRESULT on failure
     */
    ::etl::expected<uint16_t, HRESULT> get(uint16_t methodId, GetterCallback const attrCb) noexcept
    {
        return _getMethod.callMethod(*_proxy, methodId, attrCb);
    }

    /**
     * Invalidates all futures for pending attribute getter requests.
     */
    void freeAll() { _getMethod.freeAll(); }

    void updateTimeouts() { _getMethod.updateTimeouts(); }

    /**
     * Cancels a pending attribute getter request.
     *
     * \param requestId Request identifier returned from get()
     * \return HRESULT Result code
     */
    HRESULT cancelGetterRequest(uint16_t requestId) { return _getMethod.cancelRequest(requestId); }

    /**
     * Matches attribute getter responses to pending futures and invokes callbacks.
     *
     * \param msg Middleware message containing the attribute response
     */
    void answerGetterRequest(Message const& msg) { _getMethod.answerRequest(msg); }

protected:
    ProxyBase* _proxy = nullptr;

private:
    GetterMethod _getMethod;
};

/**
 * Partial specialization for proxy attributes with no getter dispatcher (void traits).
 */
template<>
class ProxyAttributeBase<void>
{
public:
    explicit ProxyAttributeBase(ProxyBase&) {}
};

} // namespace middleware::core
