/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
