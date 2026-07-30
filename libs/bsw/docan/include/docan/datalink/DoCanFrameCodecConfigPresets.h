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

    // presets for ISO 15765-2 extended addressing, which reserves 1 byte at the start of the
    // payload for the address extension byte (N_TA); otherwise identical to their non-EA
    // counterparts above.
    static DoCanFrameCodecConfig<uint8_t> const EA_PADDED_CLASSIC;
    static DoCanFrameCodecConfig<uint8_t> const EA_PADDED_FD;
    static DoCanFrameCodecConfig<uint8_t> const EA_OPTIMIZED_CLASSIC;
    static DoCanFrameCodecConfig<uint8_t> const EA_OPTIMIZED_FD;
};
} // namespace docan
