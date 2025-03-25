// Copyright 2024 Accenture.

#pragma once

#include <platform/estdint.h>

namespace docan
{
/**
 * frame sizing/padding config
 * \tparam FrameSize datatype used to represent a frame size
 */
template<class FrameSize>
struct DoCanFrameCodecConfig
{
    using FrameSizeType = FrameSize;

    struct SizeConfig
    {
        FrameSizeType _min;
        FrameSizeType _max;
    };

    SizeConfig _singleFrameSize;
    SizeConfig _firstFrameSize;
    SizeConfig _consecutiveFrameSize;
    SizeConfig _flowControlFrameSize;
    uint8_t _filler;
    uint8_t _offset;
};
} // namespace docan
