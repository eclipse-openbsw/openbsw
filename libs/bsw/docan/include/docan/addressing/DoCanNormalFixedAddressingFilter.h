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

#include "docan/addressing/DoCanNormalFixedAddressing.h"
#include "docan/addressing/IDoCanAddressConverter.h"

#include <can/filter/MaskFilter.h>
#include <util/format/StringWriter.h>
#include <util/stream/StringBufferOutputStream.h>

#include <etl/algorithm.h>
#include <etl/span.h>

#include <platform/estdint.h>

namespace docan
{
/**
 * Filter class that allows filtering and mapping of CAN addresses to transport addresses and
 * back as needed for ISO 15765-2 normal fixed addressing (29 bit CAN identifiers).
 *
 * Since the source and target address are directly encoded in the CAN identifier (see
 * DoCanNormalFixedAddressing), no address lookup table is required; the raw source/target
 * address bytes (0-255) are used directly as transport addresses. This also means the filter
 * does not need to know its "own" address: it merely extracts the target address (N_TA) from
 * a received frame and passes it on, which allows using it e.g. in a gateway routing between
 * buses without being tied to a specific node address.
 *
 * A physical (1:1) request is valid unless its target address happens to be one of the
 * configured functional addresses (that value is reserved for functional addressing). A
 * functional (1:n) request is valid only if its target address is one of the configured
 * functional addresses. Responses (and flow control frames) are always sent using physical
 * addressing, since ISO 14229 requires functional requests to be single-frame only.
 *
 * The transport target address reported upstream for a functional request is always the raw
 * functional address byte taken from the wire, unmodified - this filter does not attempt to
 * remap or canonicalize it in any way, keeping it usable standalone (e.g. in a gateway) and
 * independent of any particular upstream transport/UDS stack. Whether a given target address is
 * to be treated as a functional (broadcast) address rather than a specific node's address is
 * entirely up to the message receiver to decide (e.g. by comparing it against the set of
 * configured functional addresses it cares about) - this filter's only job is reporting the
 * address bytes exactly as transmitted on the wire.
 *
 * \tparam DataLinkLayer data link layer type
 */
template<class DataLinkLayer>
class DoCanNormalFixedAddressingFilter
: public IDoCanAddressConverter<DataLinkLayer>
, public ::can::MaskFilter
{
public:
    using DataLinkLayerType       = DataLinkLayer;
    using FrameCodecType          = DoCanFrameCodec<DataLinkLayerType>;
    using DataLinkAddressType     = typename DataLinkLayer::AddressType;
    using DataLinkAddressPairType = typename DataLinkLayerType::AddressPairType;
    using AddressingType          = DoCanNormalFixedAddressing<
        typename DataLinkLayerType::MessageSizeType,
        typename DataLinkLayerType::FrameSizeType,
        typename DataLinkLayerType::FrameIndexType>;

    using FunctionalAddressSliceType = ::etl::span<uint8_t const>;
    using TesterAddressSliceType     = ::etl::span<uint8_t const>;

    /**
     * Default constructor. Call init() to later initialize the filter.
     */
    DoCanNormalFixedAddressingFilter();

    /**
     * Initialize the filter with a list of functional addresses and the (single, shared) codec
     * used for all connections.
     * \param functionalAddresses list of valid addresses used for functional addressing.
     * \param allowedTesters list of valid addresses used for source addresses.
     * \param codec codec used to encode/decode frames.
     */
    explicit DoCanNormalFixedAddressingFilter(
        FunctionalAddressSliceType functionalAddresses,
        TesterAddressSliceType allowedTesters,
        FrameCodecType const& codec);

    /**
     * Check whether a tester/source address is allowed.
     * \param testerAddress tester address from received frame.
     * \return true if the tester address is accepted.
     */
    bool isAllowedTester(uint8_t const testerAddress) const;

    /**
     * Initialize the filter with a list of functional addresses and the (single, shared) codec
     * used for all connections.
     * \param functionalAddresses list of valid addresses used for functional addressing.
     * \param allowedTesters list of valid addresses used for source addresses. An empty list is
     *        a valid configuration meaning "no restriction - accept any tester", not an
     *        unconfigured/error state.
     * \param codec codec used to encode/decode frames.
     */
    void init(
        FunctionalAddressSliceType functionalAddresses,
        TesterAddressSliceType allowedTesters,
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
    bool isFunctionalAddress(uint8_t address) const;
    static bool toAddressByte(uint16_t transportId, uint8_t& address);

    FunctionalAddressSliceType _functionalAddresses;
    /// List of allowed tester (source) addresses. Empty means no restriction - all testers
    /// are accepted (see getReceptionParameters()).
    TesterAddressSliceType _allowedTesters;
    FrameCodecType const* _codec;
};

/**
 * Inline implementation.
 */
template<class DataLinkLayer>
DoCanNormalFixedAddressingFilter<DataLinkLayer>::DoCanNormalFixedAddressingFilter()
: MaskFilter(), _functionalAddresses(), _allowedTesters(), _codec(nullptr)
{}

template<class DataLinkLayer>
DoCanNormalFixedAddressingFilter<DataLinkLayer>::DoCanNormalFixedAddressingFilter(
    FunctionalAddressSliceType const functionalAddresses,
    TesterAddressSliceType const allowedTesters,
    FrameCodecType const& codec)
: DoCanNormalFixedAddressingFilter()
{
    init(functionalAddresses, allowedTesters, codec);
}

template<class DataLinkLayer>
bool DoCanNormalFixedAddressingFilter<DataLinkLayer>::isAllowedTester(
    uint8_t const testerAddress) const
{
    return ::etl::find(_allowedTesters.begin(), _allowedTesters.end(), testerAddress)
           != _allowedTesters.end();
}

template<class DataLinkLayer>
void DoCanNormalFixedAddressingFilter<DataLinkLayer>::init(
    FunctionalAddressSliceType const functionalAddresses,
    TesterAddressSliceType const allowedTesters,
    FrameCodecType const& codec)
{
    _functionalAddresses = functionalAddresses;
    _allowedTesters      = allowedTesters;
    _codec               = &codec;

    uint32_t const physicalBase
        = AddressingType::pack(AddressingType::PHYSICAL_ADDRESSING_ID_BASE, 0x00U, 0x00U);
    uint32_t const functionalBase
        = AddressingType::pack(AddressingType::FUNCTIONAL_ADDRESSING_ID_BASE, 0x00U, 0x00U);
    // target/source address bytes (bits 0-15) may take any value.
    MaskFilter::add(physicalBase, physicalBase | 0xFFFFU);
    MaskFilter::add(functionalBase, functionalBase | 0xFFFFU);
}

template<class DataLinkLayer>
typename DoCanNormalFixedAddressingFilter<DataLinkLayer>::FrameCodecType const*
DoCanNormalFixedAddressingFilter<DataLinkLayer>::getTransmissionParameters(
    DoCanTransportAddressPair const& transportAddressPair,
    DataLinkAddressPairType& dataLinkAddressPair) const
{
    uint8_t sourceAddress;
    uint8_t targetAddress;
    if (!toAddressByte(transportAddressPair.getSourceId(), sourceAddress)
        || !toAddressByte(transportAddressPair.getTargetId(), targetAddress))
    {
        return nullptr;
    }

    // transmission is always physical, functional requests are single-frame only and therefore
    // never require this connection-oriented (multi-frame/flow-control) path.
    dataLinkAddressPair = DataLinkAddressPairType(
        AddressingType::pack(
            AddressingType::PHYSICAL_ADDRESSING_ID_BASE, sourceAddress, targetAddress),
        AddressingType::pack(
            AddressingType::PHYSICAL_ADDRESSING_ID_BASE, targetAddress, sourceAddress));
    return _codec;
}

template<class DataLinkLayer>
typename DoCanNormalFixedAddressingFilter<DataLinkLayer>::FrameCodecType const*
DoCanNormalFixedAddressingFilter<DataLinkLayer>::getReceptionParameters(
    DataLinkAddressType const receptionAddress,
    DoCanTransportAddressPair& transportAddressPair,
    DataLinkAddressType& transmissionAddress) const
{
    uint32_t const addressingIdBase = AddressingType::addressingIdBaseOf(receptionAddress);
    uint8_t const targetAddress     = AddressingType::targetAddressOf(receptionAddress);
    uint8_t const sourceAddress     = AddressingType::sourceAddressOf(receptionAddress);
    bool isFunctional               = false;

    // Reject requests from testers that are not configured.
    // Returning nullptr makes the request silently disappear
    // before UDS processing starts.
    if (!_allowedTesters.empty())
    {
        if (!isAllowedTester(sourceAddress))
        {
            return nullptr;
        }
    }

    if (addressingIdBase == AddressingType::PHYSICAL_ADDRESSING_ID_BASE)
    {
        if (isFunctionalAddress(targetAddress))
        {
            // the target address byte is reserved for functional addressing in this
            // configuration.
            return nullptr;
        }
    }
    else if (addressingIdBase == AddressingType::FUNCTIONAL_ADDRESSING_ID_BASE)
    {
        if (!isFunctionalAddress(targetAddress))
        {
            return nullptr;
        }
        isFunctional = true;
    }
    else
    {
        return nullptr;
    }

    // the reported target address is always the raw target address byte taken from the wire,
    // unmodified; it is up to the message receiver to recognize it as a functional (broadcast)
    // address if it needs to.
    transportAddressPair = DoCanTransportAddressPair(sourceAddress, targetAddress);
    // ISO 15765-2 forbids multi-frame requests to a functional (broadcast) target, so a
    // functional request deliberately reports an invalid transmission address rather than a
    // real one: DoCanReceiver rejects any multi-frame request whose transmission address is
    // invalid, while single-frame functional requests remain unaffected, since the actual
    // response is always addressed independently, once the request has been fully received.
    transmissionAddress
        = isFunctional
              ? DataLinkLayerType::INVALID_ADDRESS
              : AddressingType::pack(
                  AddressingType::PHYSICAL_ADDRESSING_ID_BASE, sourceAddress, targetAddress);
    return _codec;
}

template<class DataLinkLayer>
char const* DoCanNormalFixedAddressingFilter<DataLinkLayer>::formatDataLinkAddress(
    DataLinkAddressType const address, ::etl::span<char> const& buffer) const
{
    ::util::stream::StringBufferOutputStream stream(buffer);
    ::util::format::StringWriter writer(stream);
    (void)writer.printf(
        "0x%02x/0x%02x",
        AddressingType::targetAddressOf(address),
        AddressingType::sourceAddressOf(address));
    return stream.getString();
}

template<class DataLinkLayer>
bool DoCanNormalFixedAddressingFilter<DataLinkLayer>::match(uint32_t const filterId) const
{
    return MaskFilter::match(filterId);
}

template<class DataLinkLayer>
bool DoCanNormalFixedAddressingFilter<DataLinkLayer>::isFunctionalAddress(
    uint8_t const address) const
{
    return ::etl::find(_functionalAddresses.begin(), _functionalAddresses.end(), address)
           != _functionalAddresses.end();
}

template<class DataLinkLayer>
bool DoCanNormalFixedAddressingFilter<DataLinkLayer>::toAddressByte(
    uint16_t const transportId, uint8_t& address)
{
    if (transportId > 0xFFU)
    {
        return false;
    }
    address = static_cast<uint8_t>(transportId);
    return true;
}
} // namespace docan
