// Copyright 2024 Accenture.

#ifndef GUARD_5DD10918_7EF9_49B8_9111_5250F6878361
#define GUARD_5DD10918_7EF9_49B8_9111_5250F6878361

#include <docan/datalink/DoCanFrameCodecConfig.h>

#include <platform/estdint.h>

namespace docan
{
/**
 * sizing/padding config presets
 */
struct DoCanFrameCodecConfigPresets
{
    // all known presets use u8 as a frame size for now

    static DoCanFrameCodecConfig<uint8_t> const PADDED_CLASSIC;
    static DoCanFrameCodecConfig<uint8_t> const PADDED_FD;
    static DoCanFrameCodecConfig<uint8_t> const OPTIMIZED_CLASSIC;
    static DoCanFrameCodecConfig<uint8_t> const OPTIMIZED_FD;
};
} // namespace docan

#endif
