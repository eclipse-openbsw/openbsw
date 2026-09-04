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

#include "someip/IEventProvider.h"
#include "someip/RequestContext.h"
#include "someip/RpcClosure.h"
#include "someip/ServiceInfo.h"

#include <ip/IPEndpoint.h>

#include <etl/ipool.h>
#include <cstdint>

namespace someip
{
/**
 * Base for generated service handler.
 */
class ServiceHandler : public IEventProvider
{
public:
    using RpcCallback     = BoundCallback<RequestContext, ServiceResultCode, ServiceResultCode>;
    using RpcCallbackPool = ::etl::ipool;

    static ::ip::IPEndpoint const& getEndpoint(CallDoneClosure const& done);

    static void setOperationResult(CallDoneClosure& done, uint8_t resultCode);

    void dispatchMethod(
        uint16_t methodId,
        ISomeIpSerializable const* pRequest,
        ISomeIpSerializable* pResponse,
        CallDoneClosure& done);

    virtual void
    onEventGroupSubscriptionStateChanged(uint16_t /* eventId */, bool /* isSubscribed */)
    {}

    virtual ISomeIpSerializable* getResponse(uint16_t methodId) = 0;

    virtual ISomeIpSerializable* getRequest(uint16_t methodId) = 0;

    virtual MethodDetail const* getMethodDetail(uint16_t methodId) const = 0;

    ISomeIpSerializable* createRequest(uint16_t methodId, SomeIpParser& parser);

    bool hasAvailableCallback() const;

    RpcCallback& getCallback();

    void releaseCallback(RpcCallback const& cb) noexcept;

protected:
    explicit ServiceHandler(RpcCallbackPool& callbackPool);

    virtual void callMethod(
        uint16_t methodId,
        ISomeIpSerializable const* pRequest,
        ISomeIpSerializable* pResponse,
        CallDoneClosure& done)
        = 0;

    static bool isSerializableValid(ISomeIpSerializable const* obj, CallDoneClosure& done);

    static bool isRequestResponseValid(
        ISomeIpSerializable const* request,
        ISomeIpSerializable const* response,
        CallDoneClosure& done);

private:
    RpcCallbackPool& _rpcCallbackPool;
};

/*
 * inline implementation
 */
inline ServiceHandler::ServiceHandler(RpcCallbackPool& callbackPool)
: _rpcCallbackPool(callbackPool)
{}

inline bool ServiceHandler::hasAvailableCallback() const { return !_rpcCallbackPool.full(); }

inline ServiceHandler::RpcCallback& ServiceHandler::getCallback()
{
    return *_rpcCallbackPool.create<RpcCallback>();
}

inline void ServiceHandler::releaseCallback(RpcCallback const& cb) noexcept
{
    _rpcCallbackPool.release(&cb);
}

} // namespace someip
