/********************************************************************************
 * Copyright (c) 2025 Roland Reichwein
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "ethernet/RxBuffers.h"

#include "ethernet/EthernetLogger.h"
#include "interrupts/SuspendResumeAllInterruptsScopedLock.h"
#include "mcu/mcu.h"

#include <etl/error_handler.h>

using namespace ::util::logger;

namespace ethernet
{

uint8_t freeRxDescriptorIndex(
    size_t const descriptorIndex, uint8_t const nextBusy, ::etl::span<pbuf*> const pbufAtIndex)
{
    return 0;
}

void freeCustomPbufHelper(pbuf* const p) {}

void RxBuffers::init() {}

void RxBuffers::interrupt() {}

static bool isErrornousBuffer(ENET_ERXD const& descriptor) { return false; }

pbuf* RxBuffers::getCurrentBuffer() { return {}; }

void handleErrornousBuffer(pbuf*& pFrame, pbuf* const pCurrentBuffer) {}

pbuf* RxBuffers::readFrame(netif*& /*pNetif*/) { return {}; }

void RxBuffers::handleBuffer(pbuf*& pFrame, pbuf* const pCurrentBuffer) const {}

void RxBuffers::handleLastBuffer(
    pbuf*& pFrame, pbuf*& pCompleteFrame, pbuf* const pCurrentBuffer) const
{}

} // namespace ethernet
