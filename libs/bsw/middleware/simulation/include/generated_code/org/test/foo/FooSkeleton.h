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

#include "org/test/foo/FooCommon.h"

namespace org::test::foo::Foo::skeleton
{

// ATTRIBUTES
using FooDefaultAttribute = ::middleware::core::SkeletonAttribute<FooStruct, internal::event_FooDefault_id, true>;
using FooDefaultGetterResponsePolicy =
    ::middleware::core::ResponseTraits<FooDefaultAttribute::AttributeType, internal::get_FooDefault_id>;


// EVENTS

class FooSkeleton : public ::middleware::core::SkeletonBase
{
  public:
    using Base = ::middleware::core::SkeletonBase;
    // METHODS

    FooSkeleton();
    ~FooSkeleton() override;

    FooSkeleton(const FooSkeleton& other) = delete;
    FooSkeleton(FooSkeleton&& other) = delete;
    FooSkeleton& operator=(const FooSkeleton& other) = delete;
    FooSkeleton& operator=(FooSkeleton&& other) = delete;

    ::middleware::core::HRESULT init(uint16_t const instanceId);
    void deInit();

    uint16_t getServiceId() const override;

    // METHODS FIRE_AND_FORGET

    // METHODS

    // ATTRIBUTES
    // FooDefault ATTRIBUTE
    virtual void get_FooDefaultAttribute(::middleware::core::ResponseBufferBase::SkeletonResponseInfo& response);
    ::middleware::core::HRESULT cancelGet_FooDefaultAttributeResponse(
        ::middleware::core::ResponseBufferBase::SkeletonResponseInfo& response);

    virtual void set_FooDefaultAttribute(const FooStruct& fooDefault);


    // ATTRIBUTES
    ::middleware::core::HRESULT respond(::middleware::core::ResponseBufferBase::SkeletonResponseInfo& response,
                                        const FooStruct& result,
                                        const bool handleResponseFailure = true);

    // ATTRIBUTES
    FooDefaultAttribute fooDefault;

    // EVENTS

  protected:
    uint32_t getProcessId() const override;
    ::middleware::core::HRESULT onNewMessageReceived(::middleware::core::Message const& msg) override;

  private:
    ::middleware::core::HRESULT sendMessage(::middleware::core::Message& msg) const final;

    // ATTRIBUTES
    ::middleware::core::ResponseBuffer<FooDefaultGetterResponsePolicy, 1U> fGet_FooDefaultAttributeResponseBuffer;

    // METHODS

    uint32_t fProcessId;

    // METHODS

    // METHODS FIRE_AND_FORGET

    // ATTRIBUTES
    ::middleware::core::HRESULT get_FooDefaultAttributeOnNewMessageReceived(
        const ::middleware::core::Message& msg);
    ::middleware::core::HRESULT set_FooDefaultAttributeOnNewMessageReceived(
        const ::middleware::core::Message& msg);
};

}  // namespace org::test::foo::Foo::skeleton
