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

#include <features/communication/dummy_serviceCommon.h>
#include <features/communication/dummy_serviceSkeleton.h>

#include <middleware/core/ResponseBufferBase.h>

#include <cstdint>

namespace application
{

namespace
{
constexpr uint32_t BROADCAST_VALUE = 0x00FF00FFU;
} // namespace

// [service-skeleton-wrapper-start]
class SkeletonApp : public features::communication::DummyService::skeleton::DummyServiceSkeleton
{
public:
    using Base = features::communication::DummyService::skeleton::DummyServiceSkeleton;
    using Foo  = features::communication::DummyService::Foo;
    using Baz  = features::communication::DummyService::Baz;
    using FireAndForgetPayload = features::communication::DummyService::FireAndForgetPayload;
    using MwInstanceId         = features::communication::DummyService::internal::InstanceId;
    using SkeletonResponseInfo = ::middleware::core::ResponseBufferBase::SkeletonResponseInfo;

    bool init()
    {
        return Base::init(MwInstanceId::InstanceId_1) == ::middleware::core::HRESULT::Ok;
    }

    void shutdown() { Base::deInit(); }

    // [service-skeleton-method-start]
    void asyncMethod([[maybe_unused]] Foo const& input, SkeletonResponseInfo& response) override
    {
        Base::asyncMethod(input, response);
        pendingAsyncResponse_ = &response;
    }

    // [service-skeleton-method-end]

    // [service-skeleton-fire-and-forget-method-start]
    void fireAndForgetMethod(FireAndForgetPayload const& payload) override
    {
        Base::fireAndForgetMethod(payload);
    }

    // [service-skeleton-fire-and-forget-method-end]

    // [service-skeleton-execute-start]
    void execute()
    {
        if (Base::isInitialized() && pendingAsyncResponse_ != nullptr)
        {
            [[maybe_unused]] ::middleware::core::HRESULT const result = Base::respondAsyncMethod(
                *pendingAsyncResponse_, features::communication::DummyService::Baz{9U, 8U, 7U, 6U});
            pendingAsyncResponse_ = nullptr;
        }
    }

    // [service-skeleton-execute-end]

    // [service-skeleton-broadcast-start]
    void publishBroadcast()
    {
        if (Base::isInitialized())
        {
            [[maybe_unused]] ::middleware::core::HRESULT const result
                = this->simpleBroadcast.send(BROADCAST_VALUE);
        }
    }

    // [service-skeleton-broadcast-end]

    // [service-skeleton-attribute-broadcast-start]
    void publishAttribute(uint32_t const value)
    {
        if (Base::isInitialized())
        {
            this->simpleField.set(value);
            [[maybe_unused]] ::middleware::core::HRESULT const result = this->simpleField.send();
        }
    }

    // [service-skeleton-attribute-broadcast-end]

    // [service-skeleton-attribute-get-start]
    void getSimpleFieldAttribute(SkeletonResponseInfo& response) override
    {
        Base::getSimpleFieldAttribute(response);
    }

    // [service-skeleton-attribute-get-end]

    // [service-skeleton-attribute-set-start]
    void setSimpleFieldAttribute(uint32_t const& value) override
    {
        Base::setSimpleFieldAttribute(value);
    }

    // [service-skeleton-attribute-set-end]

private:
    SkeletonResponseInfo* pendingAsyncResponse_{};
};

// [service-skeleton-wrapper-end]

} // namespace application
