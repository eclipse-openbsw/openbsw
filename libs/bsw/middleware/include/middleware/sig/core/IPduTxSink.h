/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "middleware/sig/core/PduAccess.h"

#include <etl/span.h>

namespace middleware::sig::core
{

/**
 * \brief Platform-provided sink that accepts the outbound PDUs of the PDU broker connection.
 *
 * Only the transmit direction goes through this interface; PDUs received from the bus are handed
 * to the connection with onReceive(). PDUs are handed over in commit order. Runtime dispatch is
 * used because the concrete bus driver is only known at integration time; calls happen at the bus
 * frame rate, not on a hot path.
 */
class IPduTxSink
{
public:
    IPduTxSink(IPduTxSink const&)            = delete;
    IPduTxSink& operator=(IPduTxSink const&) = delete;
    IPduTxSink(IPduTxSink&&)                 = delete;
    IPduTxSink& operator=(IPduTxSink&&)      = delete;
    virtual ~IPduTxSink()                    = default;

    /**
     * \brief Hands a PDU over to the sink.
     *
     * The sink takes over the PDU. It owns the pending transmission, its backlog and what happens
     * when the PDU cannot be sent right away (queueing, retry, acknowledgement, drop policy), and
     * it is responsible for making drops observable. The connection never retries and never holds
     * a PDU back, so a slow or failing destination cannot block the PDUs behind it.
     *
     * \param id PDU ID.
     * \param payload snapshot of the PDU, backed by the connection's reusable scratch buffer. It is
     * only valid during the call, so the sink must consume or copy it before returning.
     */
    virtual void submit(uint32_t id, ::etl::span<uint8_t const> payload) = 0;

protected:
    IPduTxSink() = default;
};

} // namespace middleware::sig::core
