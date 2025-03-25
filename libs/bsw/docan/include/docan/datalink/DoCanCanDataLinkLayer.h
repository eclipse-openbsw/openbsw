// Copyright 2024 Accenture.

#pragma once

#include "docan/datalink/DoCanDataLinkLayer.h"

#include <can/canframes/CANFrame.h>

namespace docan
{
/**
 * helper class holding types necessary to represent ISO transport.
 * \tparam Address datatype used to represent a data link address
 * \tparam MessageSize datatype used to represent a message size
 * \tparam FrameSize datatype used to represent a frame size
 * \tparam InvalidAddress value that represents an invalid data link address
 */

template<typename MessageSize, typename FrameSize, typename FrameIndex = uint16_t>
using DoCanCanDataLinkLayer
    = DoCanDataLinkLayer<uint32_t, MessageSize, FrameSize, ::can::CanId::INVALID_ID, FrameIndex>;
} // namespace docan
