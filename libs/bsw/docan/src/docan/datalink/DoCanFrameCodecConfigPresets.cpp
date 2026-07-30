/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "docan/datalink/DoCanFrameCodecConfigPresets.h"

namespace docan
{
DoCanFrameCodecConfig<uint8_t> const DoCanFrameCodecConfigPresets::PADDED_CLASSIC
    = {{8U, 8U}, // SF
       {8U, 8U}, // FF
       {8U, 8U}, // CF
       {8U, 8U}, // FC
       0xCCU,    // filler
       0U};      // offset

DoCanFrameCodecConfig<uint8_t> const DoCanFrameCodecConfigPresets::PADDED_FD
    = {{8U, 64U},  // SF
       {64U, 64U}, // FF
       {8U, 64U},  // CF
       {8U, 64U},  // FC
       0xCCU,      // filler
       0U};        // offset

DoCanFrameCodecConfig<uint8_t> const DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC
    = {{0U, 8U}, // SF
       {8U, 8U}, // FF
       {0U, 8U}, // CF
       {0U, 8U}, // FC
       0xCCU,    // filler
       0U};      // offset

DoCanFrameCodecConfig<uint8_t> const DoCanFrameCodecConfigPresets::OPTIMIZED_FD
    = {{0U, 64U},  // SF
       {64U, 64U}, // FF
       {0U, 64U},  // CF
       {0U, 64U},  // FC
       0xCCU,      // filler
       0U};        // offset

DoCanFrameCodecConfig<uint8_t> const DoCanFrameCodecConfigPresets::EA_PADDED_CLASSIC
    = {{8U, 8U}, // SF
       {8U, 8U}, // FF
       {8U, 8U}, // CF
       {8U, 8U}, // FC
       0xCCU,    // filler
       1U};      // offset: 1 byte reserved for the address extension byte (N_TA)

DoCanFrameCodecConfig<uint8_t> const DoCanFrameCodecConfigPresets::EA_PADDED_FD
    = {{8U, 64U},  // SF
       {64U, 64U}, // FF
       {8U, 64U},  // CF
       {8U, 64U},  // FC
       0xCCU,      // filler
       1U};        // offset: 1 byte reserved for the address extension byte (N_TA)

DoCanFrameCodecConfig<uint8_t> const DoCanFrameCodecConfigPresets::EA_OPTIMIZED_CLASSIC
    = {{0U, 8U}, // SF
       {8U, 8U}, // FF
       {0U, 8U}, // CF
       {0U, 8U}, // FC
       0xCCU,    // filler
       1U};      // offset: 1 byte reserved for the address extension byte (N_TA)

DoCanFrameCodecConfig<uint8_t> const DoCanFrameCodecConfigPresets::EA_OPTIMIZED_FD
    = {{0U, 64U},  // SF
       {64U, 64U}, // FF
       {0U, 64U},  // CF
       {0U, 64U},  // FC
       0xCCU,      // filler
       1U};        // offset: 1 byte reserved for the address extension byte (N_TA)
} // namespace docan
