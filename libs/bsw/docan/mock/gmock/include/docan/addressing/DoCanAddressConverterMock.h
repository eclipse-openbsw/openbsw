// Copyright 2024 Accenture.

#pragma once

#include "docan/addressing/IDoCanAddressConverter.h"

#include <gmock/gmock.h>

namespace docan
{
/**
 * Interface for conversion of transport address pairs into data link address pairs and back.
 * \tparam DataLinkLayer class providing data link functionality
 */
template<class DataLinkLayer>
class DoCanAddressConverterMock : public IDoCanAddressConverter<DataLinkLayer>
{
public:
    using DataLinkLayerType       = DataLinkLayer;
    using FrameCodecType          = DoCanFrameCodec<DataLinkLayerType>;
    using DataLinkAddressType     = typename DataLinkLayerType::AddressType;
    using DataLinkAddressPairType = typename DataLinkLayerType::AddressPairType;

    MOCK_METHOD(
        FrameCodecType const*,
        getTransmissionParameters,
        (DoCanTransportAddressPair const&, DataLinkAddressPairType&),
        (const, override));

    MOCK_METHOD(
        FrameCodecType const*,
        getReceptionParameters,
        (DataLinkAddressType, DoCanTransportAddressPair&, DataLinkAddressType&),
        (const, override));

    MOCK_METHOD(
        char const*,
        formatDataLinkAddress,
        (DataLinkAddressType, ::etl::span<char> const&),
        (const, override));
};

} // namespace docan
