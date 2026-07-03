/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/
/**
  *
  *  __  __ _     _     _ _
  * |  \/  (_) __| | __| | | _____      ____ _ _ __ ___
  * | |\/| | |/ _` |/ _` | |/ _ \ \ /\ / / _` | '__/ _ \
  * | |  | | | (_| | (_| | |  __/\ V  V / (_| | | |  __/
  * |_|  |_|_|\__,_|\__,_|_|\___| \_/\_/ \__,_|_|  \___|
  *
  * WARNING!
  * This file is generated. Do not edit manually.
  *
  */
#pragma once

#include <etl/delegate.h>

#include "org/test/foo/FooCommon.h"

namespace org::test::foo::Foo::proxy
{

class FooProxy;

// EVENTS

// METHODS

// ATTRIBUTES
using FooDefaultGetterDispatcherTraits = middleware::core::DispatcherTraits<FooStruct, internal::get_FooDefault_id, false, 5000U>;
using FooDefaultSetterDispatcherTraits = middleware::core::DispatcherTraits<FooStruct, internal::set_FooDefault_id, false, 0U>;

using FooDefaultAttribute = ::middleware::core::ProxyAttribute<FooProxy,
                                                            FooDefaultGetterDispatcherTraits,
                                                            1U,
                                                            FooDefaultSetterDispatcherTraits,
                                                            middleware::core::AttributeType::FullyFeatured,
                                                            FooStruct>;


class FooProxy : public middleware::core::ProxyBase, public middleware::core::ITimeoutHandler
{
    // METHODS

  public:
    using Base = middleware::core::ProxyBase;

    // METHODS

    FooProxy();
    ~FooProxy() override;

    FooProxy(const FooProxy& other) = delete;
    FooProxy(FooProxy&& other) = delete;
    FooProxy& operator=(const FooProxy& other) = delete;
    FooProxy& operator=(FooProxy&& other) = delete;


    middleware::core::HRESULT init(const internal::InstanceId instanceId,
                                   ::middleware::core::ClusterId const sourceCluster);
    void deInit();
    uint16_t getServiceId() const override;

    // METHODS FIRE_AND_FORGET

    // METHODS

    // ATTRIBUTES
    FooDefaultAttribute fooDefault;

    // EVENTS

  protected:
    uint32_t fProcessId;
    middleware::core::HRESULT onNewMessageReceived(::middleware::core::Message const& msg) override;

  private:
    void futureDispatcherDeinitialization();
    void updateTimeouts() override;
    void doUpdateTimeouts();
    middleware::core::HRESULT sendMessage(::middleware::core::Message& msg) const final;

    // METHODS
};

}  // namespace org::test::foo::Foo::proxy
