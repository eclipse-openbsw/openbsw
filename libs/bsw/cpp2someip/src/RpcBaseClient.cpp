/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/RpcBaseClient.h"

#include "someip/IRpcChannel.h"

namespace someip
{
RpcBaseClient::RpcBaseClient() : _pChannel(nullptr) {}

void RpcBaseClient::connect(IRpcChannel& channel) { _pChannel = &channel; }

void RpcBaseClient::disconnect() { _pChannel = nullptr; }

bool RpcBaseClient::isConnected() const { return _pChannel != nullptr; }

void RpcBaseClient::callMethod(
    uint16_t const methodId,
    ISomeIpSerializable const* const request,
    ISomeIpSerializable* const response,
    uint8_t const interfaceVersion,
    CallDoneClosure& done,
    uint32_t const timeout)
{
    ServiceResultCode result = RPC_ERROR_NO_CHANNEL;

    {
        if (isConnected())
        {
            result = _pChannel->callMethod(
                methodId, request, interfaceVersion, response, done, timeout);
        }
    }

    if (RPC_SENT_SUCCESSFULLY != result)
    {
        done(result);
    }
}

void RpcBaseClient::callFireAndForget(
    uint16_t const methodId,
    ISomeIpSerializable const* const request,
    uint8_t const interfaceVersion,
    CallDoneClosure& done)
{
    ServiceResultCode result = RPC_ERROR_NO_CHANNEL;

    {
        if (isConnected())
        {
            result = _pChannel->callFireAndForgetMethod(methodId, request, interfaceVersion);
        }
    }

    done(result);
}

} // namespace someip
