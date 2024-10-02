// Copyright 2024 Accenture.

#ifndef GUARD_23F7E29C_0F74_4343_8A2C_3B3F930A710E
#define GUARD_23F7E29C_0F74_4343_8A2C_3B3F930A710E

#include "docan/common/DoCanTransportAddressPair.h"

namespace docan
{
template<class DataLinkLayer>
class DoCanFrameCodec;

/**
 * A class defining a transport connection between 2 devices
 */
template<class DataLinkLayer>
class DoCanConnection
{
public:
    using DataLinkLayerType       = DataLinkLayer;
    using FrameCodecType          = DoCanFrameCodec<DataLinkLayerType>;
    using DataLinkAddressPairType = typename DataLinkLayerType::AddressPairType;

    DoCanConnection(
        FrameCodecType const& codec,
        DataLinkAddressPairType const& dataLinkAddressPair,
        DoCanTransportAddressPair const& transportAddressPair);

    FrameCodecType const& getFrameCodec() const { return *_codec; }

    DataLinkAddressPairType const& getDataLinkAddressPair() const { return _dataLinkAddressPair; }

    DoCanTransportAddressPair const& getTransportAddressPair() const
    {
        return _transportAddressPair;
    }

    bool operator==(DoCanConnection const& other) const;

private:
    // codec used to encode/decode frames
    FrameCodecType const* _codec;
    /// The CAN identifiers for this transport connection.
    DataLinkAddressPairType _dataLinkAddressPair;
    /// The identifiers of the transport connection.
    DoCanTransportAddressPair _transportAddressPair;
};

template<class DataLinkLayer>
DoCanConnection<DataLinkLayer>::DoCanConnection(
    FrameCodecType const& codec,
    DataLinkAddressPairType const& dataLinkAddressPair,
    DoCanTransportAddressPair const& transportAddressPair)
: _codec(&codec)
, _dataLinkAddressPair(dataLinkAddressPair)
, _transportAddressPair(transportAddressPair)
{}

template<class DataLinkLayer>
bool DoCanConnection<DataLinkLayer>::operator==(DoCanConnection const& other) const
{
    return (_codec == other._codec) && (_dataLinkAddressPair == other._dataLinkAddressPair)
           && (_transportAddressPair == other._transportAddressPair);
}
} // namespace docan

#endif
