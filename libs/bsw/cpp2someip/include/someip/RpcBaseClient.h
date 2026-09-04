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

#include "someip/ISomeIpSerializable.h"
#include "someip/RpcClosure.h"

#include <cstdint>

namespace someip
{
class IRpcChannel;

class RpcBaseClient
{
public:
    RpcBaseClient();

    void connect(IRpcChannel& channel);
    void disconnect();
    bool isConnected() const;

protected:
    void callMethod(
        uint16_t methodId,
        ISomeIpSerializable const* request,
        ISomeIpSerializable* response,
        uint8_t interfaceVersion,
        CallDoneClosure& done,
        uint32_t timeout = 1000);

    void callFireAndForget(
        uint16_t methodId,
        ISomeIpSerializable const* request,
        uint8_t interfaceVersion,
        CallDoneClosure& done);

private:
    IRpcChannel* _pChannel;
};

} // namespace someip
