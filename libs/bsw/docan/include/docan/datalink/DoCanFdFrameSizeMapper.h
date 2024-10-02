// Copyright 2024 Accenture.

#ifndef GUARD_58DB803D_B464_4E02_9FE1_1172775DFE97
#define GUARD_58DB803D_B464_4E02_9FE1_1172775DFE97

#include "docan/datalink/IDoCanFrameSizeMapper.h"

namespace docan
{
/**
 * leaves frame size unchanged
 * \tparam FrameSize datatype used to represent a frame size
 */
template<class FrameSize>
class DoCanFdFrameSizeMapper : public IDoCanFrameSizeMapper<FrameSize>
{
public:
    using FrameSizeType = FrameSize;

    bool mapFrameSize(FrameSizeType& size) const override
    {
        if (size > 64U)
        {
            return false;
        }
        static uint8_t const sizes[65U]
            = {0U,  1U,  2U,  3U,  4U,  5U,  6U,  7U,  8U,  12U, 12U, 12U, 12U, 16U, 16U, 16U, 16U,
               20U, 20U, 20U, 20U, 24U, 24U, 24U, 24U, 32U, 32U, 32U, 32U, 32U, 32U, 32U, 32U, 48U,
               48U, 48U, 48U, 48U, 48U, 48U, 48U, 48U, 48U, 48U, 48U, 48U, 48U, 48U, 48U, 64U, 64U,
               64U, 64U, 64U, 64U, 64U, 64U, 64U, 64U, 64U, 64U, 64U, 64U, 64U, 64U};
        size = static_cast<FrameSizeType>(sizes[size]);
        return true;
    }
};
} // namespace docan

#endif
