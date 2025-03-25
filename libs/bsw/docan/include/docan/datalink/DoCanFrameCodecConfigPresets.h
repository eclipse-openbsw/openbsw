// Copyright 2024 Accenture.

#pragma once

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

