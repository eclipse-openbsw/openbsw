/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "docan/datalink/DoCanCanDataLinkLayer.h"

#include <can/canframes/CanId.h>
#include <etl/span.h>

#include <platform/estdint.h>

namespace docan
{
/**
 * helper class for encoding/decoding frame payloads with ISO 15765-2 normal fixed addressing.
 *
 * With normal fixed addressing, the network address information is fully contained in the (29
 * bit, extended) CAN identifier:
 *
 * \code
 * bit 28-16: fixed to 0x18DA (physical) or 0x18DB (functional), see ISO 15765-2
 * bit 15-8:  target address (physical) or functional address (functional)
 * bit 7-0:   source address
 * \endcode
 *
 * This class packs/unpacks these fields into/from a single (already fully qualified, i.e.
 * carrying the extended id qualifier bit of ::can::CanId) CAN identifier, so it can be used with
 * the rest of the docan address handling infrastructure (see DoCanNormalAddressing/
 * DoCanExtendedAddressing for comparison).
 *
 * \tparam MessageSize message size to be used for data link layer
 * \tparam FrameSize frame size to be used for data link layer
 */
template<class MessageSize = uint16_t, class FrameSize = uint8_t, class FrameIndex = uint16_t>
class DoCanNormalFixedAddressing
{
public:
    using DataLinkLayerType   = DoCanCanDataLinkLayer<MessageSize, FrameSize, FrameIndex>;
    using DataLinkAddressType = typename DataLinkLayerType::AddressType;

    /// fixed bits 28-16 identifying a physical (1:1) request/response, per ISO 15765-2.
    static constexpr uint32_t PHYSICAL_ADDRESSING_ID_BASE   = 0x18DA0000UL;
    /// fixed bits 28-16 identifying a functional (1:n) request, per ISO 15765-2.
    static constexpr uint32_t FUNCTIONAL_ADDRESSING_ID_BASE = 0x18DB0000UL;

    /**
     * Pack an addressing id base (PHYSICAL_ADDRESSING_ID_BASE or FUNCTIONAL_ADDRESSING_ID_BASE),
     * target and source address into a single (fully qualified, extended) CAN identifier.
     * \param addressingIdBase PHYSICAL_ADDRESSING_ID_BASE or FUNCTIONAL_ADDRESSING_ID_BASE
     * \param targetAddress target (physical) or group (functional) address byte
     * \param sourceAddress source address byte
     */
    static constexpr DataLinkAddressType
    pack(uint32_t addressingIdBase, uint8_t targetAddress, uint8_t sourceAddress);

    /**
     * Extract the addressing id base (PHYSICAL_ADDRESSING_ID_BASE or
     * FUNCTIONAL_ADDRESSING_ID_BASE) from a packed CAN identifier.
     * \param canId packed CAN identifier as returned by pack()
     */
    static constexpr uint32_t addressingIdBaseOf(DataLinkAddressType canId);

    /**
     * Extract the target (physical) or group (functional) address byte from a packed CAN
     * identifier.
     * \param canId packed CAN identifier as returned by pack()
     */
    static constexpr uint8_t targetAddressOf(DataLinkAddressType canId);

    /**
     * Extract the source address byte from a packed CAN identifier.
     * \param canId packed CAN identifier as returned by pack()
     */
    static constexpr uint8_t sourceAddressOf(DataLinkAddressType canId);

    /**
     * Decode reception address from received CAN frame. Since normal fixed addressing carries
     * the complete addressing information in the CAN identifier, this is simply the identifier
     * itself.
     * \param canId CAN identifier
     * \param payload reference to payload
     */
    static DataLinkAddressType
    decodeReceptionAddress(uint32_t canId, ::etl::span<uint8_t const> const& payload);

    /**
     * Encode transmission address into CAN frame identifier. Since normal fixed addressing
     * carries the complete addressing information in the CAN identifier, the payload is not
     * touched.
     * \param transmissionAddress transmission address to encode
     * \param canId reference to can identifier to be set
     * \param payload reference to payload to encode
     */
    static void encodeTransmissionAddress(
        DataLinkAddressType transmissionAddress,
        uint32_t& canId,
        ::etl::span<uint8_t> const& payload);
};

/**
 * Inline implementations.
 */
template<class MessageSize, class FrameSize, class FrameIndex>
constexpr
    typename DoCanNormalFixedAddressing<MessageSize, FrameSize, FrameIndex>::DataLinkAddressType
    DoCanNormalFixedAddressing<MessageSize, FrameSize, FrameIndex>::pack(
        uint32_t const addressingIdBase, uint8_t const targetAddress, uint8_t const sourceAddress)
{
    return ::can::CanId::extended(
        addressingIdBase | (static_cast<uint32_t>(targetAddress) << 8)
        | static_cast<uint32_t>(sourceAddress));
}

template<class MessageSize, class FrameSize, class FrameIndex>
constexpr uint32_t
DoCanNormalFixedAddressing<MessageSize, FrameSize, FrameIndex>::addressingIdBaseOf(
    DataLinkAddressType const canId)
{
    // mask out bits 28-16, excluding the extended id qualifier bit and target/source address.
    return canId & 0x1FFF0000UL;
}

template<class MessageSize, class FrameSize, class FrameIndex>
constexpr uint8_t DoCanNormalFixedAddressing<MessageSize, FrameSize, FrameIndex>::targetAddressOf(
    DataLinkAddressType const canId)
{
    return static_cast<uint8_t>(canId >> 8);
}

template<class MessageSize, class FrameSize, class FrameIndex>
constexpr uint8_t DoCanNormalFixedAddressing<MessageSize, FrameSize, FrameIndex>::sourceAddressOf(
    DataLinkAddressType const canId)
{
    return static_cast<uint8_t>(canId);
}

template<class MessageSize, class FrameSize, class FrameIndex>
inline typename DoCanNormalFixedAddressing<MessageSize, FrameSize, FrameIndex>::DataLinkAddressType
DoCanNormalFixedAddressing<MessageSize, FrameSize, FrameIndex>::decodeReceptionAddress(
    uint32_t const canId, ::etl::span<uint8_t const> const& /*payload*/)
{
    return canId;
}

template<class MessageSize, class FrameSize, class FrameIndex>
inline void
DoCanNormalFixedAddressing<MessageSize, FrameSize, FrameIndex>::encodeTransmissionAddress(
    DataLinkAddressType const transmissionAddress,
    uint32_t& canId,
    ::etl::span<uint8_t> const& /*payload*/)
{
    canId = transmissionAddress;
}
} // namespace docan
