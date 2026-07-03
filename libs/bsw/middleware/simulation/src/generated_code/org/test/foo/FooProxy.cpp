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
#include "org/test/foo/FooProxy.h"

#include <etl/delegate.h>
#include <etl/expected.h>
#include <etl/utility.h>
#include <middleware/os/TaskIdProvider.h>

#include "middleware/core/Future.h"
#include "middleware/core/Message.h"
#include "middleware/core/MessagePayloadBuilder.h"
#include "middleware/core/types.h"

namespace org::test::foo::Foo::proxy
{

FooProxy::FooProxy()
    : Base(),
      // ATTRIBUTES
      fooDefault(*this),
      // EVENTS
      fProcessId(::middleware::core::INVALID_TASK_ID)
{
}

FooProxy::~FooProxy()
{
    deInit();
}

uint16_t FooProxy::getServiceId() const
{
    return internal::SERVICE_ID;
}

::middleware::core::HRESULT FooProxy::init(const internal::InstanceId instanceId,
                                           ::middleware::core::ClusterId const sourceCluster)
{
    auto const database = internal::getClusterConnectionsDb();
    auto const ret = Base::initFromInstancesDatabase(
        static_cast<uint16_t>(instanceId), etl::to_underlying(sourceCluster), database);
    if ((::middleware::core::HRESULT::Ok == ret) || (::middleware::core::HRESULT::InstanceAlreadyRegistered == ret))
    {
        static_cast<middleware::core::ITimeoutClusterConnection*>(_connection)->registerTimeoutTransceiver(*this);
    }
    if (FooProxy::isInitialized())
    {
        fProcessId = ::middleware::os::getProcessId();
        if (::middleware::core::HRESULT::Ok == ret)
        {
            futureDispatcherDeinitialization();
        }
    }
    return ret;
}

void FooProxy::deInit()
{
    checkCrossThreadError(fProcessId);
    futureDispatcherDeinitialization();
    if (FooProxy::isInitialized())
    {
        static_cast<middleware::core::ITimeoutClusterConnection*>(_connection)->unregisterTimeoutTransceiver(*this);
    }
    Base::unsubscribe(internal::SERVICE_ID);
}

// METHODS

::middleware::core::HRESULT FooProxy::onNewMessageReceived(::middleware::core::Message const& msg)
{
    ::middleware::core::HRESULT ret = ::middleware::core::HRESULT::Ok;
    checkCrossThreadError(fProcessId);
    switch (msg.getHeader().memberId)
    {
        // ATTRIBUTES
        case internal::get_FooDefault_id: {
            fooDefault.answerGetterRequest(msg);
            break;
        }
        case internal::event_FooDefault_id: {
            fooDefault.setEvent_(msg);
            break;
        }
        // EVENTS
        // METHODS
        default: {
            ret = ::middleware::core::HRESULT::ServiceMemberIdNotFound;
            break;
        }
    }
    return ret;
}

void FooProxy::futureDispatcherDeinitialization()
{
    checkCrossThreadError(fProcessId);
    // ATTRIBUTES
    fooDefault.freeAll();
    // METHODS
}

void FooProxy::updateTimeouts()
{
    doUpdateTimeouts();
}

void FooProxy::doUpdateTimeouts()
{
    checkCrossThreadError(fProcessId);
    // ATTRIBUTES
    fooDefault.updateTimeouts();
    // METHODS
}

// METHODS

// METHODS FIRE_AND_FORGET

::middleware::core::HRESULT FooProxy::sendMessage(::middleware::core::Message& msg) const
{
    checkCrossThreadError(fProcessId);
    return Base::sendMessage(msg);
}

}  // namespace org::test::foo::Foo::proxy
