// Copyright 2024 Accenture.

#pragma once

#include "docan/addressing/IDoCanAddressConverter.h"

#include <can/filter/BitFieldFilter.h>
#include <etl/span.h>
#include <util/format/StringWriter.h>
#include <util/stream/StringBufferOutputStream.h>

#include <platform/estdint.h>

#include <algorithm>

namespace docan
{
/**
 * Simple entry type for map entries of a normal addressing filter.
 */
template<class DataLinkLayer>
struct DoCanNormalAddressingFilterAddressEntry
{
    using DataLinkLayerType   = DataLinkLayer;
    using FrameCodecType      = DoCanFrameCodec<DataLinkLayerType>;
    using DataLinkAddressType = typename DataLinkLayerType::AddressType;
    using CodecIdxType        = uint8_t; // index into the codec search table

    /// The CAN identifier to listen for receiving frames for this transport connection.
    DataLinkAddressType _canReceptionId;
    /// The CAN identifier to set for transmitting frames for this transport connection.
    DataLinkAddressType _canTransmissionId;
    /// The source identifier of the transport connection.
    uint16_t _transportSourceId;
    /// The target identifier of the transport connection.
    uint16_t _transportTargetId;
    // Codec used to encode/decode frames as an RX node
    CodecIdxType _receptionCodecIdx;
    // Codec used to encode/decode frames as an TX node
    CodecIdxType _transmissionCodecIdx;
};

/**
 * Filter class that allows filtering and mapping of CAN addresses to transport addresses
 * and back as needed for normal addressing.
 * \tparam DataLinkLayer data link layer type
 * \tparam AddressEntry entry data type
 */
template<
    class DataLinkLayer,
    class AddressEntry = DoCanNormalAddressingFilterAddressEntry<DataLinkLayer>>
class DoCanNormalAddressingFilter
: public IDoCanAddressConverter<DataLinkLayer>
, public ::can::BitFieldFilter
{
public:
    using DataLinkLayerType       = DataLinkLayer;
    using FrameCodecType          = DoCanFrameCodec<DataLinkLayerType>;
    using DataLinkAddressType     = typename DataLinkLayer::AddressType;
    using DataLinkAddressPairType = typename DataLinkLayerType::AddressPairType;
    using CodecIdxType            = typename AddressEntry::CodecIdxType;

    using AddressEntryType      = AddressEntry;
    using AddressEntrySliceType = ::etl::span<AddressEntryType const>;
    using CodecsSliceType       = ::etl::span<FrameCodecType const*>;

    /**
     * Default constructor. Call init() to later initialize the filter.
     */
    DoCanNormalAddressingFilter();

    /**
     * Initialize the filter with a list of filter entries.
     * \param addressEntries list of consecutive entries ordered by the field _canReceptionId.
     * \param codecEntries codec search table
     * \note an assertion will be emitted if the frames are not in the required order!
     */
    explicit DoCanNormalAddressingFilter(
        AddressEntrySliceType addressEntries, CodecsSliceType codecEntries);

    /**
     * Initialize the filter with a list of filter entries.
     * \param addressEntries list of consecutive entries ordered by the field _canReceptionId.
     * \param codecEntries codec search table
     * \note an assertion will be emitted if the frames are not in the required order!
     */
    void init(AddressEntrySliceType addressEntries, CodecsSliceType codecEntries);

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

protected:
    /**
     * Returns frame codec associated with and ID. Null pointer is returned if no codec is found.
     */
    FrameCodecType const* getFrameCodec(CodecIdxType codecIdx) const;

private:
    static AddressEntryType const* findEntryTypeByReceptionAddress(
        DataLinkAddressType receptionAddress,
        typename AddressEntrySliceType::const_iterator beginIt,
        typename AddressEntrySliceType::const_iterator endIt);

    static bool lessReceptionId(AddressEntryType const& entry1, AddressEntryType const& entry2);

    AddressEntrySliceType _entries;
    AddressEntrySliceType _extendedEntries;
    CodecsSliceType _codecEntries;
};

/**
 * Inline implementation.
 */
template<class DataLinkLayer, class AddressEntry>
DoCanNormalAddressingFilter<DataLinkLayer, AddressEntry>::DoCanNormalAddressingFilter()
: BitFieldFilter(), _entries(), _extendedEntries(), _codecEntries()
{}

template<class DataLinkLayer, class AddressEntry>
DoCanNormalAddressingFilter<DataLinkLayer, AddressEntry>::DoCanNormalAddressingFilter(
    AddressEntrySliceType const addressEntries, CodecsSliceType const codecEntries)
: DoCanNormalAddressingFilter()
{
    init(addressEntries, codecEntries);
}

template<class DataLinkLayer, class AddressEntry>
void DoCanNormalAddressingFilter<DataLinkLayer, AddressEntry>::init(
    AddressEntrySliceType const addressEntries, CodecsSliceType const codecEntries)
{
    // Address entries must be non-zero in size
    estd_assert(addressEntries.size() != 0);
    _entries                                                          = addressEntries;
    _codecEntries                                                     = codecEntries;
    typename AddressEntrySliceType::iterator firstExtendedEntryTypeIt = _entries.end();
    uint32_t prevCanReceptionId                                       = 0U;
    typename AddressEntrySliceType::const_iterator it                 = _entries.begin();

    for (; it != _entries.end(); ++it)
    {
        DataLinkAddressType const canReceptionId = it->_canReceptionId;
        if (it != _entries.begin())
        {
            estd_assert(prevCanReceptionId < canReceptionId);
        }
        prevCanReceptionId = canReceptionId;
        if (::can::CanId::isBase(canReceptionId))
        {
            BitFieldFilter::add(canReceptionId);
        }
        else
        {
            if (firstExtendedEntryTypeIt == _entries.end())
            {
                firstExtendedEntryTypeIt = it;
            }
            if (!::can::CanId::isValid(canReceptionId))
            {
                break;
            }
        }
    }

    _extendedEntries = AddressEntrySliceType{
        firstExtendedEntryTypeIt, static_cast<size_t>(it - firstExtendedEntryTypeIt)};

    for (; it != _entries.end(); ++it)
    {
        estd_assert(!::can::CanId::isValid(it->_canReceptionId));
    }
}

template<class DataLinkLayer, class AddressEntry>
typename DoCanNormalAddressingFilter<DataLinkLayer, AddressEntry>::FrameCodecType const*
DoCanNormalAddressingFilter<DataLinkLayer, AddressEntry>::getTransmissionParameters(
    DoCanTransportAddressPair const& transportAddressPair,
    DataLinkAddressPairType& dataLinkAddressPair) const
{
    uint16_t const sourceId = transportAddressPair.getTargetId();
    uint16_t const targetId = transportAddressPair.getSourceId();
    for (AddressEntryType const& entry : _entries)
    {
        if ((entry._transportSourceId == sourceId) && (entry._transportTargetId == targetId))
        {
            dataLinkAddressPair
                = DataLinkAddressPairType(entry._canReceptionId, entry._canTransmissionId);
            return getFrameCodec(entry._transmissionCodecIdx);
        }
    }
    return nullptr;
}

template<class DataLinkLayer, class AddressEntry>
typename DoCanNormalAddressingFilter<DataLinkLayer, AddressEntry>::FrameCodecType const*
DoCanNormalAddressingFilter<DataLinkLayer, AddressEntry>::getReceptionParameters(
    DataLinkAddressType receptionAddress,
    DoCanTransportAddressPair& transportAddressPair,
    DataLinkAddressType& transmissionAddress) const
{
    // search all entries from first base entry to last extended entry.
    AddressEntryType const* const entry = findEntryTypeByReceptionAddress(
        receptionAddress, _entries.begin(), _extendedEntries.end());
    if (entry == nullptr)
    {
        return nullptr;
    }

    transmissionAddress = entry->_canTransmissionId;
    transportAddressPair
        = DoCanTransportAddressPair(entry->_transportSourceId, entry->_transportTargetId);
    return getFrameCodec(entry->_receptionCodecIdx);
}

template<class DataLinkLayer, class AddressEntry>
char const* DoCanNormalAddressingFilter<DataLinkLayer, AddressEntry>::formatDataLinkAddress(
    DataLinkAddressType const address, ::etl::span<char> const& buffer) const
{
    ::util::stream::StringBufferOutputStream stream(buffer);
    ::util::format::StringWriter writer(stream);
    (void)writer.printf("0x%08x", address);
    return stream.getString();
}

template<class DataLinkLayer, class AddressEntry>
bool DoCanNormalAddressingFilter<DataLinkLayer, AddressEntry>::match(uint32_t const filterId) const
{
    if (::can::CanId::isBase(filterId))
    {
        return BitFieldFilter::match(filterId);
    }

    return findEntryTypeByReceptionAddress(
               filterId, _extendedEntries.begin(), _extendedEntries.end())
           != nullptr;
}

template<class DataLinkLayer, class AddressEntry>
typename DoCanNormalAddressingFilter<DataLinkLayer, AddressEntry>::AddressEntryType const*
DoCanNormalAddressingFilter<DataLinkLayer, AddressEntry>::findEntryTypeByReceptionAddress(
    DataLinkAddressType const receptionAddress,
    typename AddressEntrySliceType::const_iterator const beginIt,
    typename AddressEntrySliceType::const_iterator const endIt)
{
    AddressEntryType key{};
    key._canReceptionId = receptionAddress;
    typename AddressEntrySliceType::const_iterator const it
        = std::lower_bound(beginIt, endIt, key, &lessReceptionId);
    return ((it != endIt) && (it->_canReceptionId == receptionAddress)) ? &*it : nullptr;
}

template<class DataLinkLayer, class AddressEntry>
inline bool DoCanNormalAddressingFilter<DataLinkLayer, AddressEntry>::lessReceptionId(
    AddressEntryType const& entry1, AddressEntryType const& entry2)
{
    return entry1._canReceptionId < entry2._canReceptionId;
}

template<class DataLinkLayer, class AddressEntry>
typename DoCanNormalAddressingFilter<DataLinkLayer, AddressEntry>::FrameCodecType const*
DoCanNormalAddressingFilter<DataLinkLayer, AddressEntry>::getFrameCodec(
    CodecIdxType const codecIdx) const
{
    size_t const i = static_cast<size_t>(codecIdx);
    if (i < _codecEntries.size())
    {
        return _codecEntries[i];
    }
    return nullptr;
}
} // namespace docan
