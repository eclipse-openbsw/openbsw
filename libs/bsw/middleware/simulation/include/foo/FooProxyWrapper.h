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
 * FooProxy wrapper for the simulation Core1 consumer.
 *
 * Registers a change-notification callback on the fooDefault attribute and
 * provides a helper to fire an asynchronous getter request.
 *
 * \tparam CLUSTER_ID The ClusterId of the cluster this proxy lives on.
 */
template<::middleware::core::ClusterId CLUSTER_ID>
class FooProxyWrapper
{
public:
    void init()
    {
        using namespace org::test::foo::Foo;

        proxy_.init(internal::InstanceId::InstanceId_1, CLUSTER_ID);

        proxy_.fooDefault.setReceiveHandler(
            etl::delegate<void(FooStruct const&)>::
                create<FooProxyWrapper, &FooProxyWrapper::onFooDefaultChanged>(*this));
    }

    void deInit() { proxy_.deInit(); }

    void requestGet()
    {
        using namespace org::test::foo::Foo;

        using GetterCb = etl::delegate<void(
            etl::expected<FooStruct, ::middleware::core::Future::State> const&)>;

        auto const cb = GetterCb::create<FooProxyWrapper, &FooProxyWrapper::onGetResponse>(*this);

        (void)proxy_.fooDefault.get(cb);
    }

private:
    void onFooDefaultChanged(org::test::foo::Foo::FooStruct const& value)
    {
        simulation::Logger::log(
            static_cast<uint8_t>(CLUSTER_ID), "Foo attribute changed, fooValue=", value.fooValue);
    }

    void onGetResponse(
        etl::expected<org::test::foo::Foo::FooStruct, ::middleware::core::Future::State> const&
            result)
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
};
