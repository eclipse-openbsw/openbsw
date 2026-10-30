/********************************************************************************
 * Copyright (c) 2025 Roland Reichwein
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "ethernet/TxBuffers.h"

#include "bsp/timer/SystemTimer.h"
#include "ethernet/EthernetLogger.h"
#include "etl/array.h"
#include "interrupts/SuspendResumeAllInterruptsScopedLock.h"
#include "mcu/mcu.h"

#include "util/estd/big_endian.h"
#include <etl/error_handler.h>

#include <cstring>

using namespace ::util::logger;

namespace ethernet
{
uint8_t const LENGTH_VLAN_TAG   = 4U;
uint16_t const MAX_FRAME_LENGTH = 1518U + LENGTH_VLAN_TAG;

void TxBuffers::freeDescriptor(uint8_t const n) {}

void TxBuffers::init() {}

/**
 * Commit all the frame's buffers to be sent after preparing them with
 * writeBuffer().
 * \param n         Number of buffers that compose a frame.
 * \param indexes   Array of buffer descriptor indexes.
 * \return          true, if buffers were committed, false otherwise.
 */
bool commitFrame(
    uint8_t const n, uint8_t const* const indexes, ::etl::span<ENET_ETXD> const txDescriptor)
{
    return false;
}

bool TxBuffers::ethernetWrite(::etl::span<uint8_t const> const data, DataSentCallback const onSent)
{
    return false;
}

bool TxBuffers::writeBuffer(
    uint8_t const txDescriptorIndex,
    ::etl::span<uint8_t const> const data,
    bool const lastBufferInFrame)
{
    return true;
}

bool TxBuffers::writeFrame(uint16_t const vlanId, const struct pbuf* const buf) { return false; }

bool TxBuffers::getNextDescriptorIndex(uint8_t const n, ::etl::span<uint8_t> const indexes)
{
    return true;
}

void TxBuffers::interrupt() {}

} // namespace ethernet
