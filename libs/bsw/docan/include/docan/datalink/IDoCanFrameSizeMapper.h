// Copyright 2024 Accenture.

#ifndef GUARD_39217DF8_1B24_4B5C_8EAB_F8BCB6DAD83A
#define GUARD_39217DF8_1B24_4B5C_8EAB_F8BCB6DAD83A

#include <platform/estdint.h>

namespace docan
{
/**
 * maps the frame size to the minimum appropriate
 * \tparam FrameSize datatype used to represent a frame size
 */
template<class FrameSize>
class IDoCanFrameSizeMapper
{
public:
    using FrameSizeType = FrameSize;

    virtual bool mapFrameSize(FrameSizeType& size) const = 0;

private:
    IDoCanFrameSizeMapper& operator=(IDoCanFrameSizeMapper const&) = delete;
};
} // namespace docan

#endif
