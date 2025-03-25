// Copyright 2024 Accenture.

#pragma once

#include "docan/common/DoCanTransportAddressPair.h"

#include <estd/slice.h>

namespace docan
{
template<class DataLinkLayer>
class DoCanFrameCodec;

/**
 * Interface for conversion of transport address pairs into data link address pairs and back.
 * \tparam DataLinkLayer class providing data link functionality
 */
template<class DataLinkLayer>
class IDoCanAddressConverter
{
public:
    using DataLinkLayerType       = DataLinkLayer;
    using FrameCodecType          = DoCanFrameCodec<DataLinkLayerType>;
    using DataLinkAddressType     = typename DataLinkLayerType::AddressType;
    using DataLinkAddressPairType = typename DataLinkLayerType::AddressPairType;

    /**
     * Get the corresponding transmission parameters from the transport address pair.
     * The transmissionAddress can then be used to send data frames, the receptionAddress
     * to identify received flow control frames, and frame codec to encode/decode frames on the
     * specific connection.
     * \param transportAddressPair reference to transport address pair holding source/target address
     * \param dataLinkAddressPair data link address pair holding corresponding
     *        transmission/reception addresses to use on data link layer. The address pair is not
     *        updated if there's no corresponding address pair.
     * \return frame codec used for the destination device. Null pointer is returned on error.
     */
    virtual FrameCodecType const* getTransmissionParameters(
        DoCanTransportAddressPair const& transportAddressPair,
        DataLinkAddressPairType& dataLinkAddressPair) const
        = 0;

    /**
     * Get the corresponding transport address pair and the corresponding transmission
     * address to a given reception address. The transport address pair is used for
     * acquiring transport messages and the transmission address can be used to send
     * flow control frames.
     * \param receptionAddress address to get the corresponding transport address pair for
     * \param transportAddressPair transport address pair holding corresponding source/target
     *        addresses to use on transport layer. The returned address pair is not updated if
     *        there's no corresponding transport address pair.
     * \param transmissionAddress reference to variable that receives transmission address
     *        if reception address is valid
     * \return frame codec used for the source device. Null pointer is returned on error.
     */
    virtual FrameCodecType const* getReceptionParameters(
        DataLinkAddressType receptionAddress,
        DoCanTransportAddressPair& transportAddressPair,
        DataLinkAddressType& transmissionAddress) const
        = 0;

    /**
     * Format data link address into readable string (used for logging).
     * \param address data link address to format
     * \param buffer reference to buffer that may be used for writing the string
     * \return zero terminated string
     */
    virtual char const*
    formatDataLinkAddress(DataLinkAddressType address, ::estd::slice<char> const& buffer) const
        = 0;

private:
    IDoCanAddressConverter& operator=(IDoCanAddressConverter const&) = delete;
};

} // namespace docan
