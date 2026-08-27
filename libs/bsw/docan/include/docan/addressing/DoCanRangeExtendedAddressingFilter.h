/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "docan/addressing/DoCanExtendedAddressing.h"
#include "docan/addressing/IDoCanAddressConverter.h"

#include <can/filter/IntervalFilter.h>
#include <util/format/StringWriter.h>
#include <util/stream/StringBufferOutputStream.h>

#include <etl/algorithm.h>
#include <etl/error_handler.h>
#include <etl/span.h>

#include <platform/estdint.h>

namespace docan
{
/**
 * Filter class that allows filtering and mapping of CAN addresses to transport addresses and back
 * as needed for ISO 15765-2 extended addressing, for the special (but common) case where a
 * contiguous range of CAN identifiers maps arithmetically onto a contiguous range of transport
 * addresses: canId = getBaseCanId() + (transportId - getBaseTransportId()).
 *
 * Compared to DoCanExtendedAddressingFilter, this avoids the need for an explicit lookup table
 * (and the memory it requires) at the cost of only supporting such a linear, single-range
 * mapping; an ::can::IntervalFilter is used for match(), which is cheaper than a BitFieldFilter
 * for a contiguous range of ids.
 *
 * The transport target address reported upstream for a received request is always the raw
 * target address (the address extension byte) taken from the wire, unmodified - this filter
 * does not attempt to remap or canonicalize it in any way. Whether a given target address is to
 * be treated as a functional (broadcast) address rather than a specific node's address is
 * entirely up to the message receiver to decide (e.g. by comparing it against the set of
 * configured functional addresses it cares about); the \p functionalAddresses passed to init()
 * are used by this filter only to detect (and reject) multi-frame requests aimed at a functional
 * target, as required by ISO 15765-2.
 *
 * \tparam DataLinkLayer data link layer type
 */
template<class DataLinkLayer>
class DoCanRangeExtendedAddressingFilter
: public IDoCanAddressConverter<DataLinkLayer>
, public ::can::IntervalFilter
{
public:
    using DataLinkLayerType       = DataLinkLayer;
    using FrameCodecType          = DoCanFrameCodec<DataLinkLayerType>;
    using DataLinkAddressType     = typename DataLinkLayer::AddressType;
    using DataLinkAddressPairType = typename DataLinkLayerType::AddressPairType;
    using AddressingType          = DoCanExtendedAddressing<
        typename DataLinkLayerType::MessageSizeType,
        typename DataLinkLayerType::FrameSizeType,
        typename DataLinkLayerType::FrameIndexType>;

    /// slice of transport addresses treated as functional/broadcast (see init()). ISO 15765-2
    /// does not guarantee a single functional address per node, so a whole span is accepted
    /// rather than just one value.
    using FunctionalAddressSliceType = ::etl::span<uint16_t const>;

    /**
     * Default constructor. Call init() to later initialize the filter.
     */
    DoCanRangeExtendedAddressingFilter();

    /**
     * Initialize the filter with a range of CAN identifiers and transport addresses.
     * \param baseCanId first (raw, base) CAN identifier of the range.
     * \param baseTransportId transport address corresponding to baseCanId.
     * \param count number of consecutive CAN identifiers/transport addresses in the range.
     * \param functionalAddresses transport addresses treated as functional/broadcast: ISO
     * 15765-2 forbids multi-frame requests to a functional target, so a request targeting one of
     * these addresses is reported with an invalid transmission address, causing DoCanReceiver to
     * reject it if it is a multi-frame request. This has no effect on single-frame requests,
     * whose (always physical) reply is addressed independently.
     * \param codec codec used to encode/decode frames for all addresses in the range.
     * \note an assertion will be emitted if the range does not fit into a base (11 bit) CAN
     * identifier or into an address extension byte.
     */
    DoCanRangeExtendedAddressingFilter(
        DataLinkAddressType baseCanId,
        uint16_t baseTransportId,
        uint16_t count,
        FunctionalAddressSliceType functionalAddresses,
        FrameCodecType const& codec);

    /**
     * Initialize the filter with a range of CAN identifiers and transport addresses.
     * \param baseCanId first (raw, base) CAN identifier of the range.
     * \param baseTransportId transport address corresponding to baseCanId.
     * \param count number of consecutive CAN identifiers/transport addresses in the range.
     * \param functionalAddresses transport addresses treated as functional/broadcast: ISO
     * 15765-2 forbids multi-frame requests to a functional target, so a request targeting one of
     * these addresses is reported with an invalid transmission address, causing DoCanReceiver to
     * reject it if it is a multi-frame request. This has no effect on single-frame requests,
     * whose (always physical) reply is addressed independently.
     * \param codec codec used to encode/decode frames for all addresses in the range.
     * \note an assertion will be emitted if the range does not fit into a base (11 bit) CAN
     * identifier or into an address extension byte.
     */
    void init(
        DataLinkAddressType baseCanId,
        uint16_t baseTransportId,
        uint16_t count,
        FunctionalAddressSliceType functionalAddresses,
        FrameCodecType const& codec);

    FrameCodecType const* getTransmissionParameters(
        DoCanTransportAddressPair const& transportAddressPair,
        DataLinkAddressPairType& dataLinkAddressPair) const override;

    FrameCodecType const* getReceptionParameters(
        DataLinkAddressType receptionAddress,
        DoCanTransportAddressPair& transportAddressPair,
        DataLinkAddressType& transmissionAddress) const override;

    char const* formatDataLinkAddress(
        DataLinkAddressType address, ::etl::span<char> const& buffer) const override;

    bool match(uint32_t filterId) const override;

private:
    bool transportIdToCanId(uint16_t transportId, DataLinkAddressType& canId) const;
    bool canIdToTransportId(DataLinkAddressType canId, uint16_t& transportId) const;
    bool isFunctionalAddress(uint16_t transportId) const;

    DataLinkAddressType _baseCanId;
    uint16_t _baseTransportId;
    uint16_t _count;
    FrameCodecType const* _codec;
    FunctionalAddressSliceType _functionalAddresses;
};

/**
 * Inline implementation.
 */
template<class DataLinkLayer>
DoCanRangeExtendedAddressingFilter<DataLinkLayer>::DoCanRangeExtendedAddressingFilter()
: IntervalFilter()
, _baseCanId(0U)
, _baseTransportId(0U)
, _count(0U)
, _codec(nullptr)
, _functionalAddresses()
{}

template<class DataLinkLayer>
DoCanRangeExtendedAddressingFilter<DataLinkLayer>::DoCanRangeExtendedAddressingFilter(
    DataLinkAddressType const baseCanId,
    uint16_t const baseTransportId,
    uint16_t const count,
    FunctionalAddressSliceType const functionalAddresses,
    FrameCodecType const& codec)
: DoCanRangeExtendedAddressingFilter()
{
    init(baseCanId, baseTransportId, count, functionalAddresses, codec);
}

template<class DataLinkLayer>
void DoCanRangeExtendedAddressingFilter<DataLinkLayer>::init(
    DataLinkAddressType const baseCanId,
    uint16_t const baseTransportId,
    uint16_t const count,
    FunctionalAddressSliceType const functionalAddresses,
    FrameCodecType const& codec)
{
    ETL_ASSERT(count != 0U, ETL_ERROR_GENERIC("range must not be empty"));
    ETL_ASSERT(
        (baseCanId + (count - 1U)) <= ::can::CanId::MAX_RAW_BASE_ID,
        ETL_ERROR_GENERIC("range must fit into a base (11 bit) CAN identifier"));
    ETL_ASSERT(
        (static_cast<uint32_t>(baseTransportId) + (count - 1U)) <= 0xffU,
        ETL_ERROR_GENERIC("range must fit into an address extension byte"));

    _baseCanId           = baseCanId;
    _baseTransportId     = baseTransportId;
    _count               = count;
    _codec               = &codec;
    _functionalAddresses = functionalAddresses;
    IntervalFilter::add(baseCanId, baseCanId + (count - 1U));
}

template<class DataLinkLayer>
typename DoCanRangeExtendedAddressingFilter<DataLinkLayer>::FrameCodecType const*
DoCanRangeExtendedAddressingFilter<DataLinkLayer>::getTransmissionParameters(
    DoCanTransportAddressPair const& transportAddressPair,
    DataLinkAddressPairType& dataLinkAddressPair) const
{
    DataLinkAddressType sourceCanId;
    DataLinkAddressType targetCanId;
    if (!transportIdToCanId(transportAddressPair.getSourceId(), sourceCanId)
        || !transportIdToCanId(transportAddressPair.getTargetId(), targetCanId))
    {
        return nullptr;
    }

    dataLinkAddressPair = DataLinkAddressPairType(
        AddressingType::pack(targetCanId, transportAddressPair.getSourceId()),
        AddressingType::pack(sourceCanId, transportAddressPair.getTargetId()));
    return _codec;
}

template<class DataLinkLayer>
typename DoCanRangeExtendedAddressingFilter<DataLinkLayer>::FrameCodecType const*
DoCanRangeExtendedAddressingFilter<DataLinkLayer>::getReceptionParameters(
    DataLinkAddressType receptionAddress,
    DoCanTransportAddressPair& transportAddressPair,
    DataLinkAddressType& transmissionAddress) const
{
    DataLinkAddressType const canId = AddressingType::canIdOf(receptionAddress);
    // the address extension byte identifies the target of the received frame, i.e. this node.
    uint16_t const targetId         = AddressingType::addressExtensionOf(receptionAddress);

    uint16_t sourceId;
    if (!canIdToTransportId(canId, sourceId))
    {
        return nullptr;
    }

    DataLinkAddressType targetCanId;
    if (!transportIdToCanId(targetId, targetCanId))
    {
        return nullptr;
    }

    transportAddressPair = DoCanTransportAddressPair(sourceId, targetId);
    // ISO 15765-2 forbids multi-frame requests to a functional (broadcast) target, so such a
    // request deliberately reports an invalid transmission address rather than a real one:
    // DoCanReceiver rejects any multi-frame request whose transmission address is invalid, while
    // single-frame functional requests remain unaffected, since the actual response is always
    // addressed independently, once the request has been fully received.
    transmissionAddress  = isFunctionalAddress(targetId)
                               ? DataLinkLayerType::INVALID_ADDRESS
                               : AddressingType::pack(targetCanId, sourceId);
    return _codec;
}

template<class DataLinkLayer>
char const* DoCanRangeExtendedAddressingFilter<DataLinkLayer>::formatDataLinkAddress(
    DataLinkAddressType const address, ::etl::span<char> const& buffer) const
{
    ::util::stream::StringBufferOutputStream stream(buffer);
    ::util::format::StringWriter writer(stream);
    (void)writer.printf(
        "0x%03x/0x%02x",
        AddressingType::canIdOf(address),
        AddressingType::addressExtensionOf(address));
    return stream.getString();
}

template<class DataLinkLayer>
bool DoCanRangeExtendedAddressingFilter<DataLinkLayer>::match(uint32_t const filterId) const
{
    return IntervalFilter::match(filterId);
}

template<class DataLinkLayer>
bool DoCanRangeExtendedAddressingFilter<DataLinkLayer>::transportIdToCanId(
    uint16_t const transportId, DataLinkAddressType& canId) const
{
    if ((transportId < _baseTransportId) || (transportId >= (_baseTransportId + _count)))
    {
        return false;
    }
    canId = _baseCanId + (transportId - _baseTransportId);
    return true;
}

template<class DataLinkLayer>
bool DoCanRangeExtendedAddressingFilter<DataLinkLayer>::canIdToTransportId(
    DataLinkAddressType const canId, uint16_t& transportId) const
{
    if ((canId < _baseCanId) || (canId >= (_baseCanId + _count)))
    {
        return false;
    }
    transportId = static_cast<uint16_t>(_baseTransportId + (canId - _baseCanId));
    return true;
}

template<class DataLinkLayer>
bool DoCanRangeExtendedAddressingFilter<DataLinkLayer>::isFunctionalAddress(
    uint16_t const transportId) const
{
    return ::etl::find(_functionalAddresses.begin(), _functionalAddresses.end(), transportId)
           != _functionalAddresses.end();
}
} // namespace docan
