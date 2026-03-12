// Copyright 2024 Accenture.

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
