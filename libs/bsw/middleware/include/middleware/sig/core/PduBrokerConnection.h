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

#include "middleware/sig/core/PduBrokerConnectionBase.h"

#include <etl/array.h>

#include <cstddef>

namespace middleware::sig::core
{

/**
 * \brief Connects the bus to the PDUs in shared memory.
 *
 * PDUs received from the bus are stored in shared memory by onReceive(). Writers queue the ID
 * of each PDU they commit; this class drains the queue, so no search over shared memory for
 * pending PDUs is needed.
 *
 * \tparam IdQueue middleware::queue::Queue of uint32_t, filled by the PDU writers.
 * \tparam MaxPduSize size in bytes of the largest PDU. Use the generated
 * middleware::shm::PduBrokerConnection alias, which sets it.
 *
 * \note Thread safety: single consumer of the queue. onReceive() and processTransmissions() must
 * be called from the same context.
 */
template<typename IdQueue, size_t MaxPduSize>
class PduBrokerConnection final : public PduBrokerConnectionBase
{
public:
    /**
     * \brief Creates the connection.
     *
     * \param txSink sink for the outbound PDUs, normally the bus driver.
     * \param pdus access to all PDUs, sorted by ascending PDU ID.
     * \param idQueue queue of committed PDU IDs, filled by the PDU writers.
     */
    PduBrokerConnection(
        IPduTxSink& txSink, ::etl::span<PduAccess const> const pdus, IdQueue& idQueue)
    : PduBrokerConnectionBase(txSink, pdus), _receiver(idQueue)
    {}

    /**
     * \brief Hands the committed PDUs over to the sink in commit order.
     *
     * Every ID is removed from the queue as soon as the PDU is handed over, so a sink that is
     * busy or failing never blocks the PDUs behind it. The sink owns pending transmissions,
     * retries and drops. IDs that are not PDU IDs are discarded.
     */
    void processTransmissions()
    {
        while (!_receiver.isEmpty())
        {
            transmit(_receiver.peek(), ::etl::span<uint8_t>(_scratch));
            _receiver.advance();
        }
    }

private:
    ::etl::array<uint8_t, MaxPduSize> _scratch{};
    typename IdQueue::Receiver _receiver;
};

} // namespace middleware::sig::core
