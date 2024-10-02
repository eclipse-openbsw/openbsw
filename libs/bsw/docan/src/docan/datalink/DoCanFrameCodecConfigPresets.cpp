// Copyright 2024 Accenture.

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
} // namespace docan
