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

#include <can/filter/BitFieldFilter.h>
#include <util/format/StringWriter.h>
#include <util/stream/StringBufferOutputStream.h>

#include <etl/algorithm.h>
#include <etl/error_handler.h>
#include <etl/span.h>

#include <platform/estdint.h>

namespace docan
{
/**
 * Simple entry type for map entries of an extended addressing filter, mapping a raw (base, 11
 * bit) CAN identifier to the transport address of the node that transmits on it.
 *
 * Since with extended addressing the address extension byte carried in a frame's payload always
 * identifies the target of that specific frame, a flat table of these entries is enough to
 * resolve both directions of any connection between the participating nodes: the byte itself
 * gives the target, and looking up its CAN identifier in the table gives the corresponding source
 * (and vice versa for the opposite direction).
 */
template<class DataLinkLayer>
struct DoCanExtendedAddressingFilterAddressEntry
{
    using DataLinkLayerType   = DataLinkLayer;
    using DataLinkAddressType = typename DataLinkLayerType::AddressType;

    /// raw (base, 11 bit) CAN identifier used by _transportId to transmit.
    DataLinkAddressType _canId;
    /// transport address of the node transmitting on _canId.
    uint16_t _transportId;
};

/**
 * Filter class that allows filtering and mapping of CAN addresses to transport addresses
 * and back as needed for ISO 15765-2 extended addressing.
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
 * \tparam AddressEntry entry data type
 */
template<
    class DataLinkLayer,
    class AddressEntry = DoCanExtendedAddressingFilterAddressEntry<DataLinkLayer>>
class DoCanExtendedAddressingFilter
: public IDoCanAddressConverter<DataLinkLayer>
, public ::can::BitFieldFilter
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

    using AddressEntryType      = AddressEntry;
    using AddressEntrySliceType = ::etl::span<AddressEntryType const>;

    /// slice of transport addresses treated as functional/broadcast (see init()). ISO 15765-2
    /// does not guarantee a single functional address per node, so a whole span is accepted
    /// rather than just one value.
    using FunctionalAddressSliceType = ::etl::span<uint16_t const>;

    /**
     * Default constructor. Call init() to later initialize the filter.
     */
    DoCanExtendedAddressingFilter();

    /**
     * Initialize the filter with a list of filter entries and the (single, shared) codec used
     * for all of them.
     * \param addressEntries list of consecutive entries ordered (non-strictly) ascending by the
     * field _canId. Multiple entries may share the same _canId (e.g. to let a functional/
     * broadcast target address resolve to the same, real CAN identifier as another entry's own
     * physical target address), but must otherwise still be grouped together and in order.
     * \param functionalAddresses transport addresses treated as functional/broadcast: ISO
     * 15765-2 forbids multi-frame requests to a functional target, so a request targeting one of
     * these addresses is reported with an invalid transmission address, causing DoCanReceiver to
     * reject it if it is a multi-frame request. This has no effect on single-frame requests,
     * whose (always physical) reply is addressed independently.
     * \param codec codec used to encode/decode frames for all entries.
     * \note an assertion will be emitted if the entries are not in the required order!
     */
    explicit DoCanExtendedAddressingFilter(
        AddressEntrySliceType addressEntries,
        FunctionalAddressSliceType functionalAddresses,
        FrameCodecType const& codec);

    /**
     * Initialize the filter with a list of filter entries and the (single, shared) codec used
     * for all of them.
     * \param addressEntries list of consecutive entries ordered (non-strictly) ascending by the
     * field _canId. Multiple entries may share the same _canId (e.g. to let a functional/
     * broadcast target address resolve to the same, real CAN identifier as another entry's own
     * physical target address), but must otherwise still be grouped together and in order.
     * \param functionalAddresses transport addresses treated as functional/broadcast: ISO
     * 15765-2 forbids multi-frame requests to a functional target, so a request targeting one of
     * these addresses is reported with an invalid transmission address, causing DoCanReceiver to
     * reject it if it is a multi-frame request. This has no effect on single-frame requests,
     * whose (always physical) reply is addressed independently.
     * \param codec codec used to encode/decode frames for all entries.
     * \note an assertion will be emitted if the entries are not in the required order!
     */
    void init(
        AddressEntrySliceType addressEntries,
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
    static AddressEntryType const* findEntryByCanId(
        DataLinkAddressType canId,
        typename AddressEntrySliceType::const_iterator beginIt,
        typename AddressEntrySliceType::const_iterator endIt);

    AddressEntryType const* findEntryByTransportId(uint16_t transportId) const;

    bool isFunctionalAddress(uint16_t transportId) const;

    static bool lessCanId(AddressEntryType const& entry1, AddressEntryType const& entry2);

    AddressEntrySliceType _entries;
    FrameCodecType const* _codec;
    FunctionalAddressSliceType _functionalAddresses;
};

/**
 * Inline implementation.
 */
template<class DataLinkLayer, class AddressEntry>
DoCanExtendedAddressingFilter<DataLinkLayer, AddressEntry>::DoCanExtendedAddressingFilter()
: BitFieldFilter(), _entries(), _codec(nullptr), _functionalAddresses()
{}

template<class DataLinkLayer, class AddressEntry>
DoCanExtendedAddressingFilter<DataLinkLayer, AddressEntry>::DoCanExtendedAddressingFilter(
    AddressEntrySliceType const addressEntries,
    FunctionalAddressSliceType const functionalAddresses,
    FrameCodecType const& codec)
: DoCanExtendedAddressingFilter()
{
    init(addressEntries, functionalAddresses, codec);
}

template<class DataLinkLayer, class AddressEntry>
void DoCanExtendedAddressingFilter<DataLinkLayer, AddressEntry>::init(
    AddressEntrySliceType const addressEntries,
    FunctionalAddressSliceType const functionalAddresses,
    FrameCodecType const& codec)
{
    // Address entries must be non-zero in size
    ETL_ASSERT(
        addressEntries.size() != 0, ETL_ERROR_GENERIC("list of addresses must not be empty"));

    _entries                                          = addressEntries;
    _codec                                            = &codec;
    _functionalAddresses                              = functionalAddresses;
    DataLinkAddressType prevCanId                     = 0U;
    typename AddressEntrySliceType::const_iterator it = _entries.begin();

    for (; it != _entries.end(); ++it)
    {
        DataLinkAddressType const canId = it->_canId;
        if (it != _entries.begin())
        {
            ETL_ASSERT(
                prevCanId <= canId,
                ETL_ERROR_GENERIC("can id must be ascending (duplicates allowed)"));
        }
        prevCanId = canId;
        BitFieldFilter::add(canId);
    }
}

template<class DataLinkLayer, class AddressEntry>
typename DoCanExtendedAddressingFilter<DataLinkLayer, AddressEntry>::FrameCodecType const*
DoCanExtendedAddressingFilter<DataLinkLayer, AddressEntry>::getTransmissionParameters(
    DoCanTransportAddressPair const& transportAddressPair,
    DataLinkAddressPairType& dataLinkAddressPair) const
{
    AddressEntryType const* const sourceEntry
        = findEntryByTransportId(transportAddressPair.getSourceId());
    AddressEntryType const* const targetEntry
        = findEntryByTransportId(transportAddressPair.getTargetId());
    if ((sourceEntry == nullptr) || (targetEntry == nullptr))
    {
        return nullptr;
    }

    dataLinkAddressPair = DataLinkAddressPairType(
        AddressingType::pack(targetEntry->_canId, transportAddressPair.getSourceId()),
        AddressingType::pack(sourceEntry->_canId, transportAddressPair.getTargetId()));
    return _codec;
}

template<class DataLinkLayer, class AddressEntry>
typename DoCanExtendedAddressingFilter<DataLinkLayer, AddressEntry>::FrameCodecType const*
DoCanExtendedAddressingFilter<DataLinkLayer, AddressEntry>::getReceptionParameters(
    DataLinkAddressType receptionAddress,
    DoCanTransportAddressPair& transportAddressPair,
    DataLinkAddressType& transmissionAddress) const
{
    uint32_t const canId    = AddressingType::canIdOf(receptionAddress);
    // the address extension byte identifies the target of the received frame, i.e. this node.
    uint16_t const targetId = AddressingType::addressExtensionOf(receptionAddress);

    AddressEntryType const* const sourceEntry
        = findEntryByCanId(canId, _entries.begin(), _entries.end());
    if (sourceEntry == nullptr)
    {
        return nullptr;
    }

    AddressEntryType const* const targetEntry = findEntryByTransportId(targetId);
    if (targetEntry == nullptr)
    {
        return nullptr;
    }

    transportAddressPair = DoCanTransportAddressPair(sourceEntry->_transportId, targetId);
    // ISO 15765-2 forbids multi-frame requests to a functional (broadcast) target, so such a
    // request deliberately reports an invalid transmission address rather than a real one:
    // DoCanReceiver rejects any multi-frame request whose transmission address is invalid, while
    // single-frame functional requests remain unaffected, since the actual response is always
    // addressed independently, once the request has been fully received.
    transmissionAddress
        = isFunctionalAddress(targetId)
              ? DataLinkLayerType::INVALID_ADDRESS
              : AddressingType::pack(targetEntry->_canId, sourceEntry->_transportId);
    return _codec;
}

template<class DataLinkLayer, class AddressEntry>
char const* DoCanExtendedAddressingFilter<DataLinkLayer, AddressEntry>::formatDataLinkAddress(
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

template<class DataLinkLayer, class AddressEntry>
bool DoCanExtendedAddressingFilter<DataLinkLayer, AddressEntry>::match(
    uint32_t const filterId) const
{
    return BitFieldFilter::match(filterId);
}

template<class DataLinkLayer, class AddressEntry>
typename DoCanExtendedAddressingFilter<DataLinkLayer, AddressEntry>::AddressEntryType const*
DoCanExtendedAddressingFilter<DataLinkLayer, AddressEntry>::findEntryByCanId(
    DataLinkAddressType const canId,
    typename AddressEntrySliceType::const_iterator const beginIt,
    typename AddressEntrySliceType::const_iterator const endIt)
{
    AddressEntryType key{};
    key._canId = canId;
    typename AddressEntrySliceType::const_iterator const it
        = ::etl::lower_bound(beginIt, endIt, key, &lessCanId);
    return ((it != endIt) && (it->_canId == canId)) ? &*it : nullptr;
}

template<class DataLinkLayer, class AddressEntry>
typename DoCanExtendedAddressingFilter<DataLinkLayer, AddressEntry>::AddressEntryType const*
DoCanExtendedAddressingFilter<DataLinkLayer, AddressEntry>::findEntryByTransportId(
    uint16_t const transportId) const
{
    for (AddressEntryType const& entry : _entries)
    {
        if (entry._transportId == transportId)
        {
            return &entry;
        }
    }
    return nullptr;
}

template<class DataLinkLayer, class AddressEntry>
bool DoCanExtendedAddressingFilter<DataLinkLayer, AddressEntry>::isFunctionalAddress(
    uint16_t const transportId) const
{
    return ::etl::find(_functionalAddresses.begin(), _functionalAddresses.end(), transportId)
           != _functionalAddresses.end();
}

template<class DataLinkLayer, class AddressEntry>
inline bool DoCanExtendedAddressingFilter<DataLinkLayer, AddressEntry>::lessCanId(
    AddressEntryType const& entry1, AddressEntryType const& entry2)
{
    return entry1._canId < entry2._canId;
}
} // namespace docan
