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

#include "uds/base/Subfunction.h"
#include "uds/lifecycle/IUdsLifecycleConnector.h"

namespace uds
{
class PowerDown : public Subfunction
{
public:
    explicit PowerDown(IUdsLifecycleConnector& udsLifecycleConnector);

private:
    static uint8_t const sfImplementedRequest[2];

    virtual DiagReturnCode::Type process(
        IncomingDiagConnection& connection, uint8_t const request[], uint16_t requestLength) final;
    virtual void responseSent(IncomingDiagConnection& connection, ResponseSendResult result) final;

    IUdsLifecycleConnector& fUdsLifecycleConnector;
};

} // namespace uds
