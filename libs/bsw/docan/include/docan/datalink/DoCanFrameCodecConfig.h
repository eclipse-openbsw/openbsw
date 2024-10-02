// Copyright 2024 Accenture.

#ifndef GUARD_A409E1D2_0E91_4CFC_BFE8_D14E93297025
#define GUARD_A409E1D2_0E91_4CFC_BFE8_D14E93297025

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

#endif
