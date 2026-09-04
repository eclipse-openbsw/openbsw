/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/PeriodicRoutine.h"

#include "someip/RpcChannel.h"
#include "someip/RpcClosure.h"
#include "someip/defaultlogger.h"
#include "someip/logger.h"

using ::util::logger::SOMEIP;

::someip::ServiceResultCode
PeriodicRoutine::CallDoneClosure::operator()(::someip::ServiceResultCode code_)
{
    INFO_LOG(SOMEIP, "\"done\"-closure");
    code = code_;
    return ::someip::ServiceResultCode::RPC_POSITIVE_RESPONSE;
}

PeriodicRoutine::PeriodicRoutine(::someip::RpcChannel& channel)
: _channel(channel), _request(), _reply()
{}

void PeriodicRoutine::execute()
{
    if (_done.code != ::someip::ServiceResultCode::UNDEFINED_RESULT)
    {
        if (!_reply._timeStr.empty())
        {
            INFO_LOG(
                SOMEIP,
                "Reply received %.*s",
                static_cast<int>(_reply._timeStr.size()),
                _reply._timeStr.data());

            _reply._timeStr.clear();
        }

        (void)_channel.callMethod(1, &_request, 1U, &_reply, _done, 100);
    }
}
