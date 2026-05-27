/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "platform/estdint.h"
#include "uds/base/Subfunction.h"
#include "uds/lifecycle/IUdsLifecycleConnector.h"

namespace uds
{
class DiagDispatcher;

class HardReset : public Subfunction
{
public:
    HardReset(IUdsLifecycleConnector& udsLifecycleConnector, DiagDispatcher& diagDispatcher);

    void responseSent(IncomingDiagConnection& connection, ResponseSendResult result) override;

private:
    DiagReturnCode::Type process(
        IncomingDiagConnection& connection,
        uint8_t const request[],
        uint16_t requestLength) override;

    IUdsLifecycleConnector& fUdsLifecycleConnector;
    DiagDispatcher& fDiagDispatcher;

    static uint8_t const sfImplementedRequest[2];
};

} // namespace uds
