// Copyright 2024 Accenture.

/**
 * Contains
 * \file
 * \ingroup bspConfig
 */

#pragma once

#include "ftm/Ftm.h"

namespace bios
{
// clang-format off

const tFtm::tFtmConfiguration _cfgFtm4 = {
    /* SC            */ (0U << 16) | (1U << 3) | (1U), // 80 MHz clock /2 = 40 MHz
    /* MOD           */ 3000 - 1,
    /* CNTIN         */ 1000,
    /* MODE          */ 0,
    /* SYNC          */ 0,
    /* OUTINIT       */ 0,
    /* OUTMASK       */ 0,
    /* COMBINE       */ 0,
    /* DEADTIME      */ 0,
    /* EXTTRIG       */ 0,
    /* POL           */ 0,
    /* FILTER        */ 0,
    /* FLTCTRL       */ 0,
    /* QDCTRL        */ 0,
    /* CONF          */ 0,
    /* FLTPOL        */ 0,
    /* SYNCONF       */ 0,
    /* INVCTRL       */ 0,
    /* SWOCTRL       */ 0,
    /* PWMLOAD       */ 0,
    /* HCR           */ 0,
    /* PAIR0DEADTIME */ 0,
    /* PAIR1DEADTIME */ 0,
    /* PAIR2DEADTIME */ 0,
    /* PAIR3DEADTIME */ 0
};

// clang-format on
} // namespace bios
