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

#include "someip/Closure.h"

namespace someip
{
enum ServiceResultCode
{
    RPC_POSITIVE_RESPONSE,
    COULD_NOT_DELIVER,
    BUSY_ERROR,
    OUT_OF_MEMORY,
    RPC_SENT_SUCCESSFULLY,
    RPC_SENT_SUCCESSFULLY_NO_RESPONSE_EXPECTED,
    RPC_ERROR_NO_CHANNEL,
    RPC_TIMEOUT,
    RPC_UNDEFINED_ERROR,
    RPC_SERVICE_NOT_AVAILABLE,
    RPC_METHOD_NOT_AVAILABLE,
    RPC_WRONG_PROTOCOL_VERSION,
    RPC_WRONG_INTERFACE_VERSION,
    RPC_INVALID_PAYLOAD,
    UNDEFINED_RESULT,
};

using CallDoneClosure  = Closure<ServiceResultCode, ServiceResultCode>;
using CallDoneCallback = Callback<ServiceResultCode, ServiceResultCode>;

class CallDoneSyncClosure : public CallDoneClosure
{
public:
    CallDoneSyncClosure() = default;

    ServiceResultCode operator()(ServiceResultCode param) override;

private:
    ServiceResultCode _result = ServiceResultCode::UNDEFINED_RESULT;
};

} // namespace someip
