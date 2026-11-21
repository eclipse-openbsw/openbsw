/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "middleware/core/MiddlewarePdu.h"

#include <etl/span.h>

#include <cstddef>
#include <cstdint>

namespace middleware::core
{

/**
 * \brief Size-erased access to a PDU in shared memory.
 *
 * It keeps the PDU broker code from being instantiated per PDU size. The PDU table generated from
 * the deployment is sorted by ascending PDU ID. PDU IDs are defined in the deployment, are unique
 * and need not be sequential.
 *
 * Plain function pointers are used so that the table is constant and does not hold shared-memory
 * addresses, which differ between processes.
 */
struct PduAccess
{
    /** \brief PDU ID defined in the deployment. */
    uint32_t id;
    /**
     * \brief Copies a consistent snapshot of the PDU into a buffer.
     *
     * \p out must be at least as large as the PDU.
     * \return the part of \p out that holds the snapshot.
     */
    ::etl::span<uint8_t const> (*load)(::etl::span<uint8_t> out);
    /**
     * \brief Stores a payload in the PDU.
     *
     * \return false, without modifying the PDU, if \p in does not have the size of the PDU.
     */
    bool (*store)(::etl::span<uint8_t const> in);
};

/**
 * \brief Implements PduAccess::load and PduAccess::store for the PDU slot returned by \p Slot.
 *
 * The slot is resolved on every call, because shared-memory addresses differ between processes.
 *
 * \tparam Size size of the PDU in bytes.
 * \tparam Slot returns the PDU in shared memory.
 */
template<size_t Size, MiddlewarePdu<Size>* (*Slot)()>
struct PduSlotAccess
{
    /** \brief Copies a consistent snapshot of the PDU into the first Size bytes of \p out. */
    static ::etl::span<uint8_t const> load(::etl::span<uint8_t> const out)
    {
        auto const snapshot = out.template first<Size>();
        static_cast<void>(Slot()->load(snapshot));
        return snapshot;
    }

    /** \brief Stores \p in in the PDU; returns false, without modifying it, unless it has Size
     * bytes. */
    static bool store(::etl::span<uint8_t const> const in)
    {
        if (in.size() != Size)
        {
            return false;
        }
        Slot()->store(in.template first<Size>());
        return true;
    }
};

/** \brief Returns the entry of the PDU table for the PDU \p id in the slot returned by \p Slot. */
template<size_t Size, MiddlewarePdu<Size>* (*Slot)()>
constexpr PduAccess makePduAccess(uint32_t const id)
{
    return PduAccess{id, &PduSlotAccess<Size, Slot>::load, &PduSlotAccess<Size, Slot>::store};
}

} // namespace middleware::core
