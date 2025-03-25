// Copyright 2024 Accenture.

/**
 * Contains bus identifier class BusId.
 * \file
 * \ingroup common
 */
#pragma once

#include <estd/bitset.h>
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

