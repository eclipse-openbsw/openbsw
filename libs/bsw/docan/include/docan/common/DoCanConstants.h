// Copyright 2024 Accenture.

#ifndef GUARD_97DE7412_9915_405A_9094_435C3F8C1350
#define GUARD_97DE7412_9915_405A_9094_435C3F8C1350

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

#endif // GUARD_97DE7412_9915_405A_9094_435C3F8C1350
