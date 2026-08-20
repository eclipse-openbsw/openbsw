/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include <etl/delegate.h>
#include <etl/expected.h>
#include <features/communication/dummy_serviceCommon.h>
#include <features/communication/dummy_serviceProxy.h>
#include <features/communication/dummy_serviceSkeleton.h>
#include <cstdint>

#include "example.h"
#include <middleware/core/ResponseBufferBase.h>

#include <etl/functional.h>

namespace middleware::docs::user_examples
{
namespace DummyService = ::features::communication::DummyService;

namespace
{
constexpr uint8_t FOO_VALUE_A            = 30U;
constexpr uint8_t FOO_VALUE_B            = 70U;
constexpr uint8_t FNF_VALUE_A1           = 20U;
constexpr uint8_t FNF_VALUE_A2           = 50U;
constexpr uint8_t FNF_VALUE_B1           = 40U;
constexpr uint8_t FNF_VALUE_B2           = 90U;
constexpr uint32_t BROADCAST_VALUE       = 0x00FF00FFU;
constexpr uint32_t ATTRIBUTE_WRITE_VALUE = 0x0000002AU;
} // namespace

// [service-proxy-wrapper-start]
class DummyProxyWrapperService : public DummyService::proxy::DummyServiceProxy
{
public:
    using Base          = DummyService::proxy::DummyServiceProxy;
    using MwInstanceId  = DummyService::internal::InstanceId;
    using AttributeType = DummyService::proxy::SimpleFieldAttribute::AttributeType;
    using asyncMethodResult
        = etl::expected<etl::reference_wrapper<DummyService::Baz const>, FutureState>;
    using AttributeResult = etl::expected<etl::reference_wrapper<AttributeType const>, FutureState>;
    using MwResult        = ::etl::expected<uint16_t, HRESULT>;

    void startup()
    {
        if (Base::init(MwInstanceId::InstanceId_1, ClusterId::Core1) == HRESULT::Ok)
        {
            // [service-proxy-method-subscription-start]
            simpleBroadcast.setReceiveHandler(
                etl::make_delegate<
                    DummyProxyWrapperService,
                    &DummyProxyWrapperService::onBroadcastReceived_>(*this));
            // [service-proxy-method-subscription-end]

            // [service-attribute-subscription-start]
            simpleField.setReceiveHandler(etl::make_delegate<
                                          DummyProxyWrapperService,
                                          &DummyProxyWrapperService::onAttributeUpdated_>(*this));
            // [service-attribute-subscription-end]
        }
    }

    void shutdown() { Base::deInit(); }

    // [service-proxy-method-start]
    void requestAsyncMethod()
    {
        if (!Base::isInitialized())
        {
            return;
        }

        [[maybe_unused]] MwResult const asyncMethodRequest = Base::asyncMethod(
            DummyService::Foo{FOO_VALUE_A, FOO_VALUE_B},
            etl::make_delegate<
                DummyProxyWrapperService,
                &DummyProxyWrapperService::onAsyncMethodResponse_>(*this));
    }

    // [service-proxy-method-end]

    // [service-proxy-fire-and-forget-start]
    void requestFireAndForgetMethod()
    {
        if (!Base::isInitialized())
        {
            return;
        }

        [[maybe_unused]] MwResult const fireAndForgetRequest
            = Base::fireAndForgetMethod(DummyService::FireAndForgetPayload{
                {FNF_VALUE_A1, FNF_VALUE_A2}, {FNF_VALUE_B1, FNF_VALUE_B2}});
    }

    // [service-proxy-fire-and-forget-end]

    // [service-proxy-attribute-read-start]
    void readAttribute()
    {
        if (!Base::isInitialized())
        {
            return;
        }

        [[maybe_unused]] MwResult const getRequest
            = simpleField.get(etl::make_delegate<
                              DummyProxyWrapperService,
                              &DummyProxyWrapperService::onAttributeReadFinished_>(*this));
    }

    // [service-proxy-attribute-read-end]

    // [service-proxy-attribute-write-start]
    void writeAttribute()
    {
        if (!Base::isInitialized())
        {
            return;
        }

        [[maybe_unused]] MwResult const setRequest = simpleField.set(ATTRIBUTE_WRITE_VALUE);
    }

    // [service-proxy-attribute-write-end]

private:
    void onAsyncMethodResponse_(asyncMethodResult const& output)
    {
        if (output.has_value())
        {
            lastAsynchResult_ = output.value().get().a;
        }
    }

    // [service-proxy-broadcast-callback-start]
    void onBroadcastReceived_(uint32_t const& value) { latestBroadcast_ = value; }

    // [service-proxy-broadcast-callback-end]

    // [service-proxy-attribute-subscription-callback-start]
    void onAttributeUpdated_(AttributeType const& value) { latestAttribute_ = value; }

    // [service-proxy-attribute-subscription-callback-end]

    // [service-attribute-setter-callback-start]
    void onAttributeReadFinished_(AttributeResult const& output)
    {
        if (!output.has_value())
        {
            return;
        }

        latestAttribute_ = output.value().get() + 1U;
        simpleField.set(latestAttribute_);
    }

    // [service-attribute-setter-callback-end]

    uint8_t lastAsynchResult_{};
    uint32_t latestBroadcast_{};
    AttributeType latestAttribute_{};
};

// [service-proxy-wrapper-end]

// [service-skeleton-wrapper-start]
class DummySkeletonWrapperService : public DummyService::skeleton::DummyServiceSkeleton
{
public:
    using Base                 = DummyService::skeleton::DummyServiceSkeleton;
    using MwInstanceId         = DummyService::internal::InstanceId;
    using SkeletonResponseInfo = ::middleware::core::ResponseBufferBase::SkeletonResponseInfo;
    using AttributeType        = DummyService::skeleton::SimpleFieldAttribute::AttributeType;

    void startup() { Base::init(MwInstanceId::InstanceId_1); }

    void shutdown() { Base::deInit(); }

    // [service-skeleton-method-start]
    void asyncMethod(
        [[maybe_unused]] DummyService::Foo const& fooParam, SkeletonResponseInfo& response) override
    {
        pendingAsyncResponse_ = &response;
    }

    // [service-skeleton-method-end]

    // [service-skeleton-fire-and-forget-method-start]
    void
    fireAndForgetMethod([[maybe_unused]] DummyService::FireAndForgetPayload const& payload) override
    {}

    // [service-skeleton-fire-and-forget-method-end]

    // [service-skeleton-execute-start]
    void execute()
    {
        if (!Base::isInitialized() || pendingAsyncResponse_ == nullptr)
        {
            return;
        }

        DummyService::Baz const result{9U, 8U, 7U, 6U};
        [[maybe_unused]] HRESULT const respondResult
            = Base::respond(*pendingAsyncResponse_, result);
        pendingAsyncResponse_ = nullptr;
    }

    // [service-skeleton-execute-end]

    // [service-skeleton-broadcast-start]
    void publishBroadcast()
    {
        if (Base::isInitialized())
        {
            [[maybe_unused]] HRESULT const sendResult = simpleBroadcast.send(BROADCAST_VALUE);
        }
    }

    // [service-skeleton-broadcast-end]

    // [service-skeleton-attribute-broadcast-start]
    void publishAttribute(AttributeType value)
    {
        if (Base::isInitialized())
        {
            simpleField.set(value);
            [[maybe_unused]] HRESULT const sendResult = simpleField.send();
        }
    }

    // [service-skeleton-attribute-broadcast-end]

    // [service-skeleton-attribute-get-start]
    void get_SimpleFieldAttribute(SkeletonResponseInfo& response) override
    {
        [[maybe_unused]] HRESULT const respondResult = Base::respond(response, simpleField.get());
    }

    // [service-skeleton-attribute-get-end]

    // [service-skeleton-attribute-set-start]
    void set_SimpleFieldAttribute(AttributeType const& requestedValue) override
    {
        simpleField.set(requestedValue);
    }

    // [service-skeleton-attribute-set-end]

private:
    SkeletonResponseInfo* pendingAsyncResponse_{};
};

// [service-skeleton-wrapper-end]

} // namespace middleware::docs::user_examples
