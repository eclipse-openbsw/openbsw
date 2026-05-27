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
 * Constants defined in DoCAN.
 */
enum class FrameType : uint8_t
{
    SINGLE,
    FIRST,
    CONSECUTIVE,
    FLOW_CONTROL,
};

enum class FlowStatus : uint8_t
{
    CTS,
    WAIT,
    OVFLW,
};

} // namespace docan
