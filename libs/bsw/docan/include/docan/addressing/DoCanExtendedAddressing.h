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
 * helper class for encoding/decoding frame payloads with ISO 15765-2 extended addressing.
 *
 * With extended addressing, the network address information is not (fully) contained in the CAN
 * identifier. Instead, the first payload byte of every frame (SF/FF/CF/FC) carries an address
 * extension byte that identifies the target of that specific frame, allowing several logical
 * connections to share the same CAN identifier. This class packs/unpacks that address extension
 * byte together with the (base, 11 bit) CAN identifier into a single data link address value, so
 * it can be used with the rest of the docan address handling infrastructure, which otherwise only
 * deals with plain CAN identifiers (see DoCanNormalAddressing for comparison).
 *
 * \tparam MessageSize message size to be used for data link layer
 * \tparam FrameSize frame size to be used for data link layer
 */
template<class MessageSize = uint16_t, class FrameSize = uint8_t, class FrameIndex = uint16_t>
class DoCanExtendedAddressing
{
public:
    using DataLinkLayerType   = DoCanCanDataLinkLayer<MessageSize, FrameSize, FrameIndex>;
    using DataLinkAddressType = typename DataLinkLayerType::AddressType;

    /**
     * Pack a raw (base, 11 bit) CAN identifier and an address extension byte into a single data
     * link address value.
     * \param canId raw base CAN identifier
     * \param addressExtension address extension byte to combine with the CAN identifier
     */
    static constexpr DataLinkAddressType pack(uint32_t canId, uint8_t addressExtension);

    /**
     * Extract the raw (base, 11 bit) CAN identifier from a packed data link address value.
     * \param packedAddress packed data link address as returned by pack()
     */
    static constexpr uint32_t canIdOf(DataLinkAddressType packedAddress);

    /**
     * Extract the address extension byte from a packed data link address value.
     * \param packedAddress packed data link address as returned by pack()
     */
    static constexpr uint8_t addressExtensionOf(DataLinkAddressType packedAddress);

    /**
     * Decode reception address from received CAN frame, combining the CAN identifier with the
     * address extension byte carried in the first payload byte.
     * \param canId CAN identifier
     * \param payload reference to payload
     */
    static DataLinkAddressType
    decodeReceptionAddress(uint32_t canId, ::etl::span<uint8_t const> const& payload);

    /**
     * Encode transmission address into CAN frame identifier and payload, splitting the packed
     * transmission address back into a raw CAN identifier and an address extension byte written
     * to the first payload byte.
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
constexpr typename DoCanExtendedAddressing<MessageSize, FrameSize, FrameIndex>::DataLinkAddressType
DoCanExtendedAddressing<MessageSize, FrameSize, FrameIndex>::pack(
    uint32_t const canId, uint8_t const addressExtension)
{
    return (static_cast<uint32_t>(addressExtension) << ::can::CanId::BASE_ID_BITS)
           | (canId & ::can::CanId::MAX_RAW_BASE_ID);
}

template<class MessageSize, class FrameSize, class FrameIndex>
constexpr uint32_t DoCanExtendedAddressing<MessageSize, FrameSize, FrameIndex>::canIdOf(
    DataLinkAddressType const packedAddress)
{
    return packedAddress & ::can::CanId::MAX_RAW_BASE_ID;
}

template<class MessageSize, class FrameSize, class FrameIndex>
constexpr uint8_t DoCanExtendedAddressing<MessageSize, FrameSize, FrameIndex>::addressExtensionOf(
    DataLinkAddressType const packedAddress)
{
    return static_cast<uint8_t>(packedAddress >> ::can::CanId::BASE_ID_BITS);
}

template<class MessageSize, class FrameSize, class FrameIndex>
inline typename DoCanExtendedAddressing<MessageSize, FrameSize, FrameIndex>::DataLinkAddressType
DoCanExtendedAddressing<MessageSize, FrameSize, FrameIndex>::decodeReceptionAddress(
    uint32_t const canId, ::etl::span<uint8_t const> const& payload)
{
    uint8_t const addressExtension = payload.empty() ? uint8_t(0U) : payload[0];
    return pack(canId, addressExtension);
}

template<class MessageSize, class FrameSize, class FrameIndex>
inline void DoCanExtendedAddressing<MessageSize, FrameSize, FrameIndex>::encodeTransmissionAddress(
    DataLinkAddressType const transmissionAddress,
    uint32_t& canId,
    ::etl::span<uint8_t> const& payload)
{
    canId = canIdOf(transmissionAddress);
    if (!payload.empty())
    {
        payload[0] = addressExtensionOf(transmissionAddress);
    }
}
} // namespace docan
