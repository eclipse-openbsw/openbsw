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

#include <etl/delegate.h>
#include <etl/expected.h>

#include "Logger.h"
#include "middleware/core/Future.h"
#include "org/test/foo/FooProxy.h"

/**
 * FooProxy wrapper for the simulation Cluster1 consumer.
 *
 * Registers a change-notification callback on the fooDefault attribute and
 * provides an asynchronous getter and application-level polling for FooMethod.
 * All operations, including middleware processing, run on the consumer thread.
 *
 * \tparam CLUSTER_ID The ClusterId of the cluster this proxy lives on.
 */
template<::middleware::core::ClusterId CLUSTER_ID>
class FooProxyWrapper
{
public:
    using FooMethodResult = org::test::foo::Foo::FooMethodResult;
    using FooMethodState  = ::middleware::core::Future::State;

    struct FooMethodPollResult
    {
        FooMethodState state;
        FooMethodResult value;
    };

    void init()
    {
        using namespace org::test::foo::Foo;

        proxy_.init(internal::InstanceId::InstanceId_1, CLUSTER_ID);

        proxy_.fooDefault.setReceiveHandler(
            ::etl::delegate<void(FooStruct const&)>::
                create<FooProxyWrapper, &FooProxyWrapper::onFooDefaultChanged>(*this));
    }

    void deInit()
    {
        proxy_.deInit();
        fooMethodState_ = FooMethodState::Invalid;
        fooMethodValue_ = {};
    }

    [[nodiscard]] ::middleware::core::HRESULT requestFooMethod(uint32_t const startValue)
    {
        using ::middleware::core::HRESULT;
        if (fooMethodState_ != FooMethodState::Invalid)
        {
            simulation::Logger::log(
                static_cast<uint8_t>(CLUSTER_ID), "FooMethod request rejected: Busy");
            return HRESULT::ServiceBusy;
        }

        using Proxy   = org::test::foo::Foo::proxy::FooProxy;
        auto const cb = Proxy::FooMethodCallback::
            template create<FooProxyWrapper, &FooProxyWrapper::onFooMethodResponse>(*this);

        fooMethodState_    = FooMethodState::Pending;
        auto const request = proxy_.fooMethod(startValue, cb);
        if (!request.has_value())
        {
            fooMethodState_ = FooMethodState::Invalid;
            simulation::Logger::log(
                static_cast<uint8_t>(CLUSTER_ID),
                "FooMethod request failed, error=",
                static_cast<unsigned>(request.error()));
            return request.error();
        }

        simulation::Logger::log(
            static_cast<uint8_t>(CLUSTER_ID), "FooMethod request sent, startValue=", startValue);
        return HRESULT::Ok;
    }

    [[nodiscard]] FooMethodPollResult pollFooMethod()
    {
        FooMethodPollResult const result{fooMethodState_, fooMethodValue_};
        if (fooMethodState_ != FooMethodState::Invalid
            && fooMethodState_ != FooMethodState::Pending)
        {
            fooMethodState_ = FooMethodState::Invalid;
            fooMethodValue_ = {};
        }
        return result;
    }

    void requestGet()
    {
        using namespace org::test::foo::Foo;

        using GetterCb = ::etl::delegate<void(::etl::expected<
                                              ::etl::reference_wrapper<FooStruct const>,
                                              ::middleware::core::Future::State> const&)>;

        auto const cb = GetterCb::create<FooProxyWrapper, &FooProxyWrapper::onGetResponse>(*this);

        (void)proxy_.fooDefault.get(cb);
    }

private:
    void onFooMethodResponse(org::test::foo::Foo::proxy::FooProxy::FooMethodResult const& result)
    {
        if (result.has_value())
        {
            // The middleware releases the response payload after this callback.
            fooMethodValue_ = result.value().get();
            fooMethodState_ = FooMethodState::Ready;
        }
        else
        {
            fooMethodState_ = result.error();
        }
    }

    void onFooDefaultChanged(org::test::foo::Foo::FooStruct const& value)
    {
        simulation::Logger::log(
            static_cast<uint8_t>(CLUSTER_ID), "Foo attribute changed, fooValue=", value.fooValue);
    }

    void onGetResponse(::etl::expected<
                       ::etl::reference_wrapper<org::test::foo::Foo::FooStruct const>,
                       ::middleware::core::Future::State> const& result)
    {
        if (result.has_value())
        {
            simulation::Logger::log(
                static_cast<uint8_t>(CLUSTER_ID),
                "Foo get response, fooValue=",
                result.value().get().fooValue);
        }
    }

    org::test::foo::Foo::proxy::FooProxy proxy_;
    FooMethodState fooMethodState_{FooMethodState::Invalid};
    FooMethodResult fooMethodValue_{};
};
