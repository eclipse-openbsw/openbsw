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
#include "org/test/foo/FooSkeleton.h"

#include <middleware/logger/Logger.h>
#include <middleware/os/TaskIdProvider.h>

#include "middleware/core/LoggerApi.h"
#include "middleware/core/Message.h"
#include "middleware/core/MessagePayloadBuilder.h"
#include "middleware/core/ResponseBufferBase.h"
#include "middleware/core/types.h"

namespace org::test::foo::Foo::skeleton
{

// METHODS FIRE_AND_FORGET

// ATTRIBUTES
middleware::core::HRESULT FooSkeleton::get_FooDefaultAttributeOnNewMessageReceived(
    const middleware::core::Message& msg)
{
    middleware::core::HRESULT ret = middleware::core::HRESULT::ServiceBusy;

    const auto response = fGet_FooDefaultAttributeResponseBuffer.getAvailableResponse(
        msg.getHeader().addressId, msg.getHeader().srcClusterId, msg.getHeader().requestId);
    if (response != nullptr)
    {
        get_FooDefaultAttribute(*response);
        ret = middleware::core::HRESULT::Ok;
    }

    return ret;
}

middleware::core::HRESULT FooSkeleton::set_FooDefaultAttributeOnNewMessageReceived(
    const middleware::core::Message& msg)
{
    const auto& payload = middleware::core::MessagePayloadBuilder::getInstance().readPayload<FooStruct>(msg);
    set_FooDefaultAttribute(payload);

    return middleware::core::HRESULT::Ok;
}


// METHODS

FooSkeleton::FooSkeleton()
    : Base(),
      // ATTRIBUTES
      fooDefault(*this),
      // EVENTS
      // ATTRIBUTES
      fGet_FooDefaultAttributeResponseBuffer(*this),
      // METHODS
      fProcessId(middleware::core::INVALID_TASK_ID)
{
}

FooSkeleton::~FooSkeleton()
{
    deInit();
}

middleware::core::HRESULT FooSkeleton::init(uint16_t const instanceId)
{
    auto const db = internal::getClusterConnectionsDb();
    const auto ret = Base::initFromInstancesDatabase(static_cast<uint16_t>(instanceId), db);
    if (Base::isInitialized())
    {
        fProcessId = middleware::os::getProcessId();
    }

    return ret;
}

void FooSkeleton::deInit()
{
    Base::checkCrossThreadError(fProcessId);
    Base::unsubscribe(internal::SERVICE_ID);
}

uint16_t FooSkeleton::getServiceId() const
{
    return internal::SERVICE_ID;
}

middleware::core::HRESULT FooSkeleton::onNewMessageReceived(middleware::core::Message const& msg)
{
    middleware::core::HRESULT ret = middleware::core::HRESULT::ServiceMemberIdNotFound;
    Base::checkCrossThreadError(fProcessId);
    switch (msg.getHeader().memberId)
    {
        // METHODS FIRE_AND_FORGET
        // ATTRIBUTES
        case internal::get_FooDefault_id: {
            ret = get_FooDefaultAttributeOnNewMessageReceived(msg);
            break;
        }
        case internal::set_FooDefault_id: {
            ret = set_FooDefaultAttributeOnNewMessageReceived(msg);
            break;
        }
        // METHODS
        default: {
            if (msg.isError() && msg.isEvent())
            {
                middleware::logger::logMessageSendingFailure(
                    middleware::logger::LogLevel::Error, middleware::logger::Error::DispatchMessage, ret, msg);
                ret = middleware::core::HRESULT::Ok;
            }
            break;
        }
    }
    return ret;
}

// METHODS

// ATTRIBUTES
middleware::core::HRESULT FooSkeleton::respond(middleware::core::ResponseBufferBase::SkeletonResponseInfo& response,
                                               const FooStruct& result,
                                               const bool handleResponseFailure)
{
    return fGet_FooDefaultAttributeResponseBuffer.respond(response, result, handleResponseFailure);
}


middleware::core::HRESULT FooSkeleton::cancelGet_FooDefaultAttributeResponse(
    middleware::core::ResponseBufferBase::SkeletonResponseInfo& response)
{
    return fGet_FooDefaultAttributeResponseBuffer.cancelResponse(response);
}


// METHODS

// ATTRIBUTES
void FooSkeleton::get_FooDefaultAttribute(middleware::core::ResponseBufferBase::SkeletonResponseInfo& response)
{
    const auto& val = fooDefault.get();
    static_cast<void>(respond(response, val));
}

void FooSkeleton::set_FooDefaultAttribute(const FooStruct& arg)
{
    fooDefault.set(arg);
}


middleware::core::HRESULT FooSkeleton::sendMessage(middleware::core::Message& msg) const
{
    Base::checkCrossThreadError(fProcessId);
    return Base::sendMessage(msg);
}

uint32_t FooSkeleton::getProcessId() const
{
    return fProcessId;
}

}  // namespace org::test::foo::Foo::skeleton
