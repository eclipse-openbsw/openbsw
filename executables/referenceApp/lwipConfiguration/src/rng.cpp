/********************************************************************************
 * Copyright (c) 2025 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "rng.h"

#include <etl/random.h>

#ifdef __cplusplus
extern "C"
{
#endif

// The implementation of rand() in the currently used clang toolchain
// (LLVM Embedded Toolchain for ARM 19.1.1 with picolibc as provided at
// https://github.com/ARM-software/LLVM-embedded-toolchain-for-Arm/)
// crashes on the S32K148 target. Therefore, use the ETL implementation
// of RNG instead.
int lwip_rand()
{
    static etl::random_lcg generator{1};
    return static_cast<int>(generator());
}

#ifdef __cplusplus
}
#endif
