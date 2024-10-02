// Copyright 2024 Accenture.

#ifndef GUARD_60B7ECB8_6564_49EF_8867_35DF38C0C1F8
#define GUARD_60B7ECB8_6564_49EF_8867_35DF38C0C1F8

#include "estd/uncopyable.h"
#include "uds/base/Subfunction.h"
#include "uds/lifecycle/IUdsLifecycleConnector.h"

namespace uds
{
class PowerDown : public Subfunction
{
    UNCOPYABLE(PowerDown);

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

#endif // GUARD_60B7ECB8_6564_49EF_8867_35DF38C0C1F8
