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

#include <middleware/core/Future.h>
#include <util/logger/Logger.h>

#include "org/test/foo/FooProxy.h"
#include <app/DemoLogger.h>

class FooProxyWrapper
{
public:
    void init()
    {
        using namespace org::test::foo::Foo;

        proxy_.init(internal::InstanceId::InstanceId_1, ::middleware::core::ClusterId::Core1);

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
        ::util::logger::Logger::info(
            ::util::logger::DEMO,
            "[Middleware] Foo attribute changed, fooValue=%u",
            value.fooValue);
    }

    void onGetResponse(
        etl::expected<org::test::foo::Foo::FooStruct, ::middleware::core::Future::State> const&
            result)
    {
        if (result.has_value())
        {
            ::util::logger::Logger::info(
                ::util::logger::DEMO,
                "[Middleware] Foo get response, fooValue=%u",
                result.value().fooValue);
        }
    }

    org::test::foo::Foo::proxy::FooProxy proxy_;
};
