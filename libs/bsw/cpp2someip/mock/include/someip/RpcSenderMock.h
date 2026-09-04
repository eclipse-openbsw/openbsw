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

#include "someip/IRpcChannel.h"
#include "someip/IRpcSender.h"

#include <gmock/gmock.h>

namespace someip
{
class RpcSenderMock : public IRpcSender
{
public:
    MOCK_METHOD(
        ServiceResultCode,
        sendRequest,
        (ISomeIpSerializable const* pRequest,
         service_id::type serviceId,
         uint16_t methodId,
         uint8_t interfaceVersion,
         bool isResponseExpected,
         IRpcChannel& channel,
         uint32_t timeout));

    MOCK_METHOD(void, requestExpired, (IRpcChannel & channel));
};

} // namespace someip
