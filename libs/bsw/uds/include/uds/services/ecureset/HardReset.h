// Copyright 2024 Accenture.

#ifndef GUARD_D15703B1_80FF_4FF1_84F4_86313F554F27
#define GUARD_D15703B1_80FF_4FF1_84F4_86313F554F27

#include "estd/uncopyable.h"
#include "platform/estdint.h"
#include "uds/base/Subfunction.h"
#include "uds/lifecycle/IUdsLifecycleConnector.h"

namespace uds
{
class DiagDispatcher2;

class HardReset : public Subfunction
{
    UNCOPYABLE(HardReset);

public:
    HardReset(IUdsLifecycleConnector& udsLifecycleConnector, DiagDispatcher2& diagDispatcher);

    void responseSent(IncomingDiagConnection& connection, ResponseSendResult result) override;

private:
    static uint16_t const RESET_TIME = 1000U; // ms

    DiagReturnCode::Type process(
        IncomingDiagConnection& connection,
        uint8_t const request[],
        uint16_t requestLength) override;

    IUdsLifecycleConnector& fUdsLifecycleConnector;
    DiagDispatcher2& fDiagDispatcher;

    static uint8_t const sfImplementedRequest[2];
};

} // namespace uds

#endif // GUARD_D15703B1_80FF_4FF1_84F4_86313F554F27
