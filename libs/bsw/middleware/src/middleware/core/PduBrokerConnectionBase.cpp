/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "middleware/core/PduBrokerConnectionBase.h"

#include "middleware/core/IPduTxSink.h"
#include "middleware/core/PduAccess.h"

#include <etl/algorithm.h>
#include <etl/span.h>

#include <cstdint>

namespace middleware::core
{

PduBrokerConnectionBase::PduBrokerConnectionBase(
    IPduTxSink& txSink, ::etl::span<PduAccess const> const& pdus)
: _txSink(txSink), _pdus(pdus)
{}

bool PduBrokerConnectionBase::onReceive(
    uint32_t const id, ::etl::span<uint8_t const> const& payload)
{
    PduAccess const* const pdu = find(id);
    return (pdu != nullptr) && pdu->store(payload);
}

void PduBrokerConnectionBase::transmit(uint32_t const id, ::etl::span<uint8_t> const& scratch)
{
    PduAccess const* const pdu = find(id);
    if (pdu == nullptr)
    {
        ++_unknownTxIds;
        return;
    }
    ::etl::span<uint8_t const> const snapshot = pdu->load(scratch);
    _txSink.submit(id, snapshot);
}

PduAccess const* PduBrokerConnectionBase::find(uint32_t const id) const
{
    auto const* const pduIt = ::etl::lower_bound(
        _pdus.begin(),
        _pdus.end(),
        id,
        [](PduAccess const& pdu, uint32_t const value) { return pdu.id < value; });
    return ((pduIt != _pdus.end()) && (pduIt->id == id)) ? pduIt : nullptr;
}

} // namespace middleware::core
