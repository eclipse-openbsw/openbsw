// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

#pragma once

#include <platform/estdint.h>

namespace platform
{

/**
 * \brief Minimal BSP static initialization for NUCLEO-F413ZH
 *
 * Handles clock configuration, UART, and system timer init.
 */
class StaticBsp
{
public:
    void init();
};

} // namespace platform
