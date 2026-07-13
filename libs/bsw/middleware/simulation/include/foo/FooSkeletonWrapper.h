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

#include "Logger.h"
#include "org/test/foo/FooSkeleton.h"

/**
 * Concrete FooSkeleton implementation for the simulation Cluster0 provider.
 *
 * Stores a FooStruct value and handles get/set requests.
 * On every set, the new value is broadcast to all subscribed proxies.
 */
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
            simulation::Logger::log(
                static_cast<uint8_t>(::middleware::core::ClusterId::Cluster0),
                "Foo broadcast sent, fooValue=",
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
        simulation::Logger::log(
            static_cast<uint8_t>(::middleware::core::ClusterId::Cluster0),
            "Foo set received, fooValue=",
            data_.fooValue);
    }

private:
    org::test::foo::Foo::FooStruct data_{};
};
