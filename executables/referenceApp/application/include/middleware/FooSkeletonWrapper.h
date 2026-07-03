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

#include <middleware/core/ResponseBufferBase.h>
#include <util/logger/Logger.h>

#include "org/test/foo/FooSkeleton.h"
#include <app/DemoLogger.h>

class FooSkeletonWrapper : public org::test::foo::Foo::skeleton::FooSkeleton
{
public:
    void init() { FooSkeleton::init(org::test::foo::Foo::internal::InstanceId::InstanceId_1); }

    void sendBroadcast()
    {
        data_.fooValue++;
        auto const result = fooDefault.send(data_);
        if (result == ::middleware::core::HRESULT::Ok)
        {
            ::util::logger::Logger::info(
                ::util::logger::DEMO,
                "[Middleware] Foo broadcast sent, fooValue=%u",
                data_.fooValue);
        }
    }

    void get_FooDefaultAttribute(
        ::middleware::core::ResponseBufferBase::SkeletonResponseInfo& response) override
    {
        respond(response, data_);
    }

    void set_FooDefaultAttribute(org::test::foo::Foo::FooStruct const& value) override
    {
        data_ = value;
        (void)fooDefault.send(data_);
        ::util::logger::Logger::info(
            ::util::logger::DEMO, "[Middleware] Foo set received, fooValue=%u", data_.fooValue);
    }

    void deInit() { FooSkeleton::deInit(); }

private:
    org::test::foo::Foo::FooStruct data_{};
};
