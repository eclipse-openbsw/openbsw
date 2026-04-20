// Copyright 2024 Accenture.
// Copyright 2026 BMW AG

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
