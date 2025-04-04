// Copyright 2024 Accenture.

#pragma once

#include "estd/uncopyable.h"
#include "platform/estdint.h"
#include "uds/base/Subfunction.h"
#include "uds/lifecycle/IUdsLifecycleConnector.h"

namespace uds
{
class DiagDispatcher2;

class SoftReset : public Subfunction
{
    UNCOPYABLE(SoftReset);

public:
    SoftReset(IUdsLifecycleConnector& udsLifecycleConnector, DiagDispatcher2& diagDispatcher);

    void responseSent(IncomingDiagConnection& connection, ResponseSendResult result) override;

private:
    static uint16_t const RESET_TIME = 1000U; // ms
    static uint8_t const sfImplementedRequest[2];

    DiagReturnCode::Type process(
        IncomingDiagConnection& connection,
        uint8_t const request[],
        uint16_t requestLength) override;

    IUdsLifecycleConnector& fUdsLifecycleConnector;
    DiagDispatcher2& fDiagDispatcher;
};

} // namespace uds
