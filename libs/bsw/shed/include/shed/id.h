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

#include <cstddef>
#include <cstdint>

namespace shed
{
struct id
{
    uint32_t idx        = 0;
    uint32_t generation = 0;

    id() = default;

    id(uint32_t const i, uint32_t const g) : idx(i), generation(g) {}

    operator size_t() const { return static_cast<size_t>(idx); }

    bool valid() const { return generation != 0; }
};

} // namespace shed
