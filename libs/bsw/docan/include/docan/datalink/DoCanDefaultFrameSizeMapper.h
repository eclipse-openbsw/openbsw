// Copyright 2024 Accenture.

#ifndef GUARD_2DE3C9D4_D952_4EDE_946C_51F601E73C25
#define GUARD_2DE3C9D4_D952_4EDE_946C_51F601E73C25

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

#endif
