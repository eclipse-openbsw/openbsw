// Copyright 2024 Accenture.

#pragma once

#include "platform/estdint.h"
#include "transport/TransportJob.h"
#include "uds/UdsConfig.h"
#include "uds/connection/IncomingDiagConnection.h"

#include <etl/algorithm.h>
#include <etl/intrusive_list.h>
#include <etl/pool.h>
#include <etl/queue.h>
#include <etl/utility.h>

namespace uds
{
struct DiagnosisConfiguration
{
    uint16_t DiagAddress;
    uint16_t const BroadcastAddress;
    uint16_t const MaxResponsePayloadSize;
    uint8_t DiagBusId;
    bool ActivateOutgoingPending;
    bool AcceptAllRequests;
    bool CopyFunctionalRequests;
    ::async::ContextType Context;
};

inline IncomingDiagConnection*
acquireIncomingDiagConnection(::etl::ipool& pool, ::async::ContextType context)
{
    if (pool.full())
    {
        return nullptr;
    }
    return pool.template create<IncomingDiagConnection>(context);
}

} // namespace uds
