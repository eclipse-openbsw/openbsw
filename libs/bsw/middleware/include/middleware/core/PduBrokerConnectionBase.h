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

#include "middleware/core/IPduTxSink.h"
#include "middleware/core/PduAccess.h"

#include <etl/span.h>

namespace middleware::core
{

/**
 * \brief Queue-independent part of PduBrokerConnection.
 *
 * \note Thread safety: not thread-safe. All calls must come from the single context that drives
 * the bus.
 */
class PduBrokerConnectionBase
{
public:
    PduBrokerConnectionBase(PduBrokerConnectionBase const&)            = delete;
    PduBrokerConnectionBase& operator=(PduBrokerConnectionBase const&) = delete;
    PduBrokerConnectionBase(PduBrokerConnectionBase&&)                 = delete;
    PduBrokerConnectionBase& operator=(PduBrokerConnectionBase&&)      = delete;

    /**
     * \brief Stores a PDU received from the bus in shared memory, which increments its
     * sequence counter.
     *
     * The PDU must not also have a writer, as there must be a single writer per PDU.
     *
     * \return false if \p id is unknown or \p payload does not match the PDU size.
     */
    bool onReceive(uint32_t id, ::etl::span<uint8_t const> const& payload);

    /**
     * \brief Returns the number of IDs taken from the ID queue that were not PDU IDs and were
     * discarded. The counter wraps around.
     */
    uint32_t getUnknownTxIds() const { return _unknownTxIds; }

protected:
    /**
     * \brief Creates the connection.
     *
     * \param txSink sink for the outbound PDUs, normally the bus driver.
     * \param pdus access to all PDUs, sorted by ascending PDU ID.
     */
    PduBrokerConnectionBase(IPduTxSink& txSink, ::etl::span<PduAccess const> const& pdus);
    ~PduBrokerConnectionBase() = default;

    /**
     * \brief Copies a consistent snapshot of the PDU \p id into \p scratch and hands it to the
     * sink. Nothing happens if \p id is not a PDU ID.
     * \param scratch buffer that is at least as large as the largest PDU; the bus never reads
     * shared memory directly.
     */
    void transmit(uint32_t id, ::etl::span<uint8_t> const& scratch);

    /** \brief Returns the access to the PDU with the given ID, or nullptr if there is none. */
    PduAccess const* find(uint32_t id) const;

    IPduTxSink& _txSink;
    ::etl::span<PduAccess const> _pdus;
    uint32_t _unknownTxIds{0U};
};

} // namespace middleware::core
