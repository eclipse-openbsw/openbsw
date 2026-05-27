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
