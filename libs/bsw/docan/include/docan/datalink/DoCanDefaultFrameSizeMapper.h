// Copyright 2024 Accenture.

#pragma once

#include "docan/datalink/IDoCanFrameSizeMapper.h"

namespace docan
{
/**
 * leaves frame size unchanged
 * \tparam FrameSize datatype used to represent a frame size
 */
template<class FrameSize>
class DoCanDefaultFrameSizeMapper : public IDoCanFrameSizeMapper<FrameSize>
{
public:
    using FrameSizeType = FrameSize;

    bool mapFrameSize(FrameSizeType& size) const override
    {
        (void)size;
        return true;
    }
};
} // namespace docan

