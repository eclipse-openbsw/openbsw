/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/ServiceHandler.h"

#include "someip/SomeIpParser.h"

namespace someip
{
// static
::ip::IPEndpoint const& ServiceHandler::getEndpoint(CallDoneClosure const& done)
{
    return static_cast<RpcCallback const&>(done).getParam1().remoteIp;
}

void ServiceHandler::setOperationResult(CallDoneClosure& done, uint8_t const resultCode)
{
    uint32_t const SOMEIP_ERROR_OFFSET = 0x1FU;
    static_cast<RpcCallback&>(done).getParam1().operationResult
        = resultCode == 0U ? 0U : static_cast<uint8_t>(resultCode + SOMEIP_ERROR_OFFSET);
}

void ServiceHandler::dispatchMethod(
    uint16_t const methodId,
    ISomeIpSerializable const* const pRequest,
    ISomeIpSerializable* const pResponse,
    CallDoneClosure& done)
{
    MethodDetail const* const details = getMethodDetail(methodId);

    if (details == nullptr)
    {
        done(::someip::RPC_METHOD_NOT_AVAILABLE);
        return;
    }

    switch (details->methodType)
    {
        case SomeIpMethodType::METHOD_NO_PARAMETERS:
        {
            callMethod(methodId, nullptr, nullptr, done);
            break;
        }
        case SomeIpMethodType::METHOD_REQUEST_RESPONSE:
        {
            if (isRequestResponseValid(pRequest, pResponse, done))
            {
                callMethod(methodId, pRequest, pResponse, done);
            }
            break;
        }
        case SomeIpMethodType::METHOD_REQUEST_ONLY:
        {
            if (isSerializableValid(pRequest, done))
            {
                callMethod(methodId, pRequest, nullptr, done);
            }
            break;
        }
        case SomeIpMethodType::METHOD_RESPONSE_ONLY:
        {
            if (isSerializableValid(pResponse, done))
            {
                callMethod(methodId, nullptr, pResponse, done);
            }
            break;
        }
        default:
        {
            done(::someip::RPC_METHOD_NOT_AVAILABLE);
            break;
        }
    }
}

ISomeIpSerializable* ServiceHandler::createRequest(uint16_t const methodId, SomeIpParser& parser)
{
    ISomeIpSerializable* const _request = getRequest(methodId);

    if (_request == nullptr)
    {
        return nullptr;
    }

    parser >> *_request;

    if (parser.isGood())
    {
        return _request;
    }
    return nullptr;
}

// static
bool ServiceHandler::isSerializableValid(
    ISomeIpSerializable const* const obj, CallDoneClosure& done)
{
    if (obj == nullptr)
    {
        done(::someip::RPC_INVALID_PAYLOAD);
        return false;
    }

    return true;
}

// static
bool ServiceHandler::isRequestResponseValid(
    ISomeIpSerializable const* const request,
    ISomeIpSerializable const* const response,
    CallDoneClosure& done)
{
    if ((request == nullptr) || (response == nullptr))
    {
        done(::someip::RPC_INVALID_PAYLOAD);
        return false;
    }

    return true;
}

} // namespace someip
