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
