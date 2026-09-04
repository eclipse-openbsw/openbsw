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

#include "async/IRunnable.h"
#include "someip/RpcChannel.h"
#include "someip/RpcClosure.h"
#include "someip/TimeServiceReply.h"
#include "someip/TimeServiceRequest.h"

class PeriodicRoutine : public async::IRunnable
{
    ::someip::RpcChannel& _channel;
    TimeServiceRequest _request;
    TimeServiceReply _reply;

    class CallDoneClosure : public ::someip::CallDoneClosure
    {
    public:
        ::someip::ServiceResultCode code{::someip::ServiceResultCode::RPC_SERVICE_NOT_AVAILABLE};

        ::someip::ServiceResultCode operator()(someip::ServiceResultCode code_) override;
    };

    CallDoneClosure _done;

public:
    explicit PeriodicRoutine(::someip::RpcChannel& channel);

    void execute() override;
};
