// Copyright 2024 Accenture.

#pragma once

#include "docan/common/DoCanJobHandle.h"
#include "docan/datalink/DoCanDataLinkAddressPair.h"

namespace docan
{
/**
 * helper class holding types necessary to represent ISO transport.
 * \tparam Address datatype used to represent a data link address
 * \tparam MessageSize datatype used to represent a message size
 * \tparam FrameSize datatype used to represent a frame size
 * \tparam InvalidAddress value that represents an invalid data link address
 * \tparam FrameIndex datatype used to represent a FrameIndex, defaults to uint16_t
 */
template<
    typename Address,
    typename MessageSize,
    typename FrameSize,
    Address InvalidAddress,
    typename FrameIndex = uint16_t>
struct DoCanDataLinkLayer
{
    using AddressType     = Address;
    using AddressPairType = DoCanDataLinkAddressPair<AddressType>;
    using MessageSizeType = MessageSize;
    using FrameSizeType   = FrameSize;
    using FrameIndexType  = FrameIndex;
    using JobHandleType   = DoCanJobHandle<uint16_t, uint16_t>;

    static AddressType const INVALID_ADDRESS = InvalidAddress;
};

} // namespace docan
