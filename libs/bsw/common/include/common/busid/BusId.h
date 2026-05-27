/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

/**
 * Contains bus identifier class BusId.
 * \file
 * \ingroup common
 */
#pragma once

#include <platform/estdint.h>

#include <type_traits>

namespace common
{
namespace busid
{

/**
 * Traits class for bus identification.
 *
 *
 */
class BusIdTraits
{
    BusIdTraits() = delete;

public:
    /**
     * Gets name of Bus.
     * \param index ID of Bus
     *
     * \return Name of Bus
     */
    static char const* getName(uint8_t index);
};

} // namespace busid
} // namespace common
