/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "ProvidedServiceData.h"
#include "ProvidedServiceImpl.h"
#include "someip/IEventSender.h"
#include "someip/RpcClosure.h"
#include "someip/ServiceHandler.h"
#include "someip/SomeIpConstants.h"

#include <etl/error_handler.h>
#include <cstdint>

template<typename CallbackPoolType, someip::port::type ServicePort>
class ProvidedServiceHandler : public someip::ServiceHandler
{
    ProvidedServiceData _request;
    ProvidedServiceData _response;
    someip::IEventSender& _eventSender;
    someip::MethodDetail const _methodDetail
        = {uint16_t(1),
           someip::SomeIpMethodType::METHOD_REQUEST_RESPONSE,
           someip::SomeIpCallSemantic::SEMANTIC_REQUEST_RESPONSE};

public:
    ProvidedServiceHandler(CallbackPoolType&, someip::IEventSender&);

    someip::ISomeIpSerializable* getResponse(uint16_t methodId) override;
    someip::ISomeIpSerializable* getRequest(uint16_t methodId) override;

    bool notifyInitialEvents(
        someip::service_id::type,
        someip::instance_id::type,
        someip::major_version::type,
        someip::eventgroup_id::type,
        ::ip::IPAddress const&,
        someip::port::type,
        someip::proto::type) override
    {
        return true;
    }

private:
    someip::MethodDetail const* getMethodDetail(uint16_t methodId) const override;
    void callMethod(
        uint16_t methodId,
        someip::ISomeIpSerializable const* pRequest,
        someip::ISomeIpSerializable* pResponse,
        someip::CallDoneClosure& done) override;
};

template<typename CallbackPoolType, someip::port::type ServicePort>
inline ProvidedServiceHandler<CallbackPoolType, ServicePort>::ProvidedServiceHandler(
    CallbackPoolType& methodCallbacks, someip::IEventSender& es)
: ServiceHandler(methodCallbacks), _request(), _response(), _eventSender(es)
{}

template<typename CallbackPoolType, someip::port::type ServicePort>
someip::ISomeIpSerializable* ProvidedServiceHandler<CallbackPoolType, ServicePort>::getResponse(
    [[maybe_unused]] uint16_t methodId)
{
    return &_response;
}

template<typename CallbackPoolType, someip::port::type ServicePort>
someip::ISomeIpSerializable* ProvidedServiceHandler<CallbackPoolType, ServicePort>::getRequest(
    [[maybe_unused]] uint16_t methodId)
{
    return &_request;
}

template<typename CallbackPoolType, someip::port::type ServicePort>
someip::MethodDetail const* ProvidedServiceHandler<CallbackPoolType, ServicePort>::getMethodDetail(
    [[maybe_unused]] uint16_t methodId) const
{
    return &_methodDetail;
}

template<typename CallbackPoolType, someip::port::type ServicePort>
void ProvidedServiceHandler<CallbackPoolType, ServicePort>::callMethod(
    uint16_t methodId,
    someip::ISomeIpSerializable const* pRequest,
    someip::ISomeIpSerializable* pResponse,
    someip::CallDoneClosure& done)
{
    ETL_ASSERT(pRequest == &_request, ETL_ERROR_GENERIC("request pointer mismatch"));
    ETL_ASSERT(pResponse == &_response, ETL_ERROR_GENERIC("response pointer mismatch"));
    ::callMethod(0xabc, _eventSender, ServicePort, methodId, &_request, &_response, done);
}
