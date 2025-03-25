// Copyright 2024 Accenture.

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

