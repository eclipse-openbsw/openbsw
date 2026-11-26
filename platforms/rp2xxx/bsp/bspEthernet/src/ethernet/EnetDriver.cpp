/********************************************************************************
 * Copyright (c) 2025 Roland Reichwein
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "ethernet/EnetDriver.h"

#include "3rdparty/nxp/S32K148.h"
#include "bsp/timer/isEqualAfterTimeout.h"
#include "ethernet/EthernetLogger.h"

#include <cstring>

extern "C"
{
#include "netif/etharp.h"
}

using namespace ::util::logger;

namespace
{
static uint8_t calculateHashIndex(uint8_t const* /* pMulticastAddr */)
{
    // TODO: implement GAUR/GALR hash CRC calculation.
    return 0;
}
} // namespace

namespace ethernet
{
constexpr uint16_t MAX_FRAME_LENGTH = 1518U;

EnetDriver::EnetDriver(
    ::etl::array<uint8_t, 6> macAddr, EnetDriver::Configuration const& configuration)
: _macAddr(macAddr)
, _rxBuffers(
      configuration.rx.fRxDescriptor,
      configuration.rx.sfPbufToRxDescriptorIndexMapping,
      configuration.rx.rxBuffers,
      configuration.rx.fRxPbufs,
      configuration.rx.RX_BUFFER_SIZE)
, _txBuffers(
      configuration.tx.referencedPbufs,
      configuration.tx.fTxDescriptor,
      configuration.tx.txIsrListeners,
      configuration.tx.descriptorIndices)
, _miiConfig(configuration.miiConfig)
{}

void cleanUpMIB() {}

uint8_t initDevice(
    ::etl::span<uint8_t const, 6> macAddr,
    uint32_t const miiConfig,
    uint16_t const rxBufferSize,
    uint32_t const rxDescriptorAddress,
    uint32_t const txDescriptorAddress)
{
    return ERR_OK;
}

uint8_t EnetDriver::init() { return 0; }

void EnetDriver::enableVlanTagging() {}

void EnetDriver::disableVlanTagging() {}

uint8_t EnetDriver::start() { return ERR_OK; }

uint8_t EnetDriver::stop() { return ERR_OK; }

void EnetDriver::interruptRx() {}

void EnetDriver::interruptError() {}

void setGroupcastAddressRecognitionImpl(uint8_t const* const mac) {}

void EnetDriver::interruptGroup0() {}

void EnetDriver::interruptGroup1() {}

void EnetDriver::interruptGroup2() {}

void EnetDriver::interruptGroup3() { interruptError(); }

void EnetDriver::setGroupcastAddressRecognition(uint8_t const* const mac) const {}

} /* namespace ethernet */

extern "C"
{
uint8_t ethernet_multicast_join(ip4_addr const* const /* ip */) { return ERR_OK; }
}
