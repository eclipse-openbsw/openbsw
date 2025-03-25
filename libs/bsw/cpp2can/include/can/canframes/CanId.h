// Copyright 2024 Accenture.

/**
 * Contains Cpp2CAN CanId.
 * \file    CanId.h
 * \ingroup cpp2can
 */
#pragma once

#include <cstdint>

// The value EXTENDED_QUALIFIER_BIT_VALUE is intentionally defined as a macro.
// This allows a check with other definitions on compiler - level.
// Do not remove or change this definition without refactoring !
#define EXTENDED_QUALIFIER_BIT_VALUE 0x80000000U

namespace can
{
/**
 * Helper class for working with base and extended CAN identifiers side-by-side.
 *
 * CAN identifiers are represented using 32 bit values. Raw CAN identifiers consume up to 29 bits
 * (for extended ids). To distinguish base identifiers, extended identifiers and to allow also
 * representation of invalid identifiers 2 additional bits are used:
 *
 * \code{.cpp}
 * 0                   1                   2                   3
 * 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      Raw id                             |F|I|X|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * \endcode
 *
 * with
 *
 * \par Raw id
 *    29 bit value holding the identifier value. Typically base identifiers will only
 *    only consume up to 11 bits. Checking raw identifier values for validity is not scope of this
 * class.
 *
 * \par F
 *    This bit indicates that the identifier is not compatible with can fd.
 *
 * \par I
 *    This bit indicates an invalid identifier if set to 1.
 *
 * \par X
 *    This bit indicates whether the identifier is an extended (1) or a base (0) identifier.
 */
class CanId
{
    CanId();

public:
    /// bit mask for extended qualifier bit.

    static constexpr uint32_t EXTENDED_QUALIFIER_BIT     = EXTENDED_QUALIFIER_BIT_VALUE;
    /// bit mask for invalid qualifier bit.
    static constexpr uint32_t INVALID_QUALIFIER_BIT      = 0x40000000U;
    /// bit mask for non fd qualifier bit.
    static constexpr uint32_t FORCE_NON_FD_QUALIFIER_BIT = 0x20000000U;
    /// invalid identifier.
    static constexpr uint32_t INVALID_ID                 = 0xffffffffU;

    /// maximum value for a base identifier (11 bits).
    static constexpr uint32_t MAX_RAW_BASE_ID     = 0x000007ffU;
    /// maximum value for an extended identifier (29 bits).
    static constexpr uint32_t MAX_RAW_EXTENDED_ID = 0x1fffffffU;

    /**
     * Helper class for retrieving a base CAN identifier at compile time.
     * \tparam RawId raw 11 bit base identifier.
     * \note The raw identifier is checked for the allowed value range (11 bits) to avoid unintended
     * typos
     */
    template<uint32_t RawId>
    struct Base
    {
        /// check value range
        static_assert(RawId <= MAX_RAW_BASE_ID, "");

        /// The represented CAN identifier value.
        static uint32_t const value = RawId;
    };

    /**
     * Helper class for retrieving an extended CAN identifier at compile time.
     * \tparam RawId raw 29 bit extended identifier
     * \note The raw identifier is checked for the allowed value range (29 bits) to avoid unintended
     * typos
     */
    template<uint32_t RawId>
    struct Extended
    {
        /// check value range
        static_assert(RawId <= MAX_RAW_EXTENDED_ID, "");

        /// The represented CAN identifier value.
        static uint32_t const value = CanId::EXTENDED_QUALIFIER_BIT | RawId;
    };

    /**
     * Helper class for retrieving a CAN identifier at compile time.
     * \tparam RawId raw identifier: 11 bit in case of normal identifier, 29 bit in case of extended
     * identifier \tparam IsExtended flag indicating whether the RawId should be interpreted as a
     * base or extended id.
     */
    template<uint32_t RawId, bool IsExtended>
    struct Id : public Base<RawId>
    {};

    template<uint32_t RawId>
    struct Id<RawId, true> : public Extended<RawId>
    {};

    /**
     * Helper class for retrieving an invalid CAN identifier at compile time.
     */
    struct Invalid
    {
        /// The represented CAN identifier value.
        static uint32_t const value = INVALID_ID;
    };

    /**
     * Convert a raw base identifier to a CAN identifier.
     * \param rawId raw base identifier (11 bit)
     * \return CAN identifier
     */
    static uint32_t base(uint16_t baseId);
    /**
     * Convert a raw extended identifier to a CAN identifier.
     * \param rawId raw extended identifier (29 bit)
     * \return CAN identifier
     */
    static uint32_t extended(uint32_t extendedId);
    /**
     * Convert a raw identifier to a CAN can identifier containing the force fd qualifier.
     * \param id raw identifier (29 bit)
     * \return new identifier with the force fd qualifiers set.
     */
    static uint32_t forceNoFd(uint32_t id);
    /**
     * Create an identifier from either base or extended identifier.
     * \param rawId raw value of CAN identifier (11 bit base or 29 bit extended)
     * \param isExtended true if the identifier is extended
     * \return CAN identifier
     */
    static uint32_t id(uint32_t value, bool isValueExtended);
    /**
     * Create an identifier with additional qualifier information.
     * \param rawId raw value of CAN identifier (11 bit base or 29 bit extended)
     * \param isExtended true if the identifier is extended
     * \param forceNoFd true if the force fd flag should be set within the identifier.
     * \return CAN identifier
     */
    static uint32_t id(uint32_t value, bool isValueExtended, bool forceNoFd);
    /**
     * Get the raw identifier value from a CAN identifier.
     * \return 29 bit identifier in case of an extended id, 11 bit base identifier otherwise
     */
    static uint32_t rawId(uint32_t value);
    /**
     * Check whether CAN identifier is a base identifier.
     * \param id identifier to check for base
     * \return true if the given identifier is a base identifier
     */
    static bool isBase(uint32_t value);
    /**
     * Check whether CAN identifier is an extended identifier.
     * \param id identifier to check for extended
     * \return true if the given identifier is a extended identifier
     */
    static bool isExtended(uint32_t value);
    /**
     * Check whether CAN identifier has the force fd qualifier set.
     * \param id identifier to check for force fd qualifier.
     * \return true if the given identifier is a force fd identifier
     */
    static bool isForceNoFd(uint32_t value);
    /**
     * Check the validity of a CAN identifier. A valid CAN identifier is either
     * a base identifier or an extended identifier.
     * \param id identifier to check for validity
     * \return true if the given identifier is valid
     */
    static bool isValid(uint32_t value);
};

template<uint32_t Id>
uint32_t const CanId::Base<Id>::value;

template<uint32_t Id>
uint32_t const CanId::Extended<Id>::value;

inline uint32_t CanId::base(uint16_t const baseId) { return static_cast<uint32_t>(baseId); }

inline uint32_t CanId::extended(uint32_t const extendedId)
{
    return extendedId | EXTENDED_QUALIFIER_BIT;
}

inline uint32_t CanId::forceNoFd(uint32_t const id) { return id | FORCE_NON_FD_QUALIFIER_BIT; }

inline uint32_t CanId::id(uint32_t const value, bool const isValueExtended)
{
    return value | (isValueExtended ? EXTENDED_QUALIFIER_BIT : 0U);
}

inline uint32_t CanId::id(uint32_t const value, bool const isValueExtended, bool const forceNoFd)
{
    return value | (isValueExtended ? EXTENDED_QUALIFIER_BIT : 0U)
           | (forceNoFd ? FORCE_NON_FD_QUALIFIER_BIT : 0U);
}

inline uint32_t CanId::rawId(uint32_t const value)
{
    return value & (~(EXTENDED_QUALIFIER_BIT | FORCE_NON_FD_QUALIFIER_BIT));
}

inline bool CanId::isValid(uint32_t const value) { return (value & INVALID_QUALIFIER_BIT) == 0U; }

inline bool CanId::isBase(uint32_t const value) { return (value & EXTENDED_QUALIFIER_BIT) == 0U; }

inline bool CanId::isExtended(uint32_t const value)
{
    return (value & EXTENDED_QUALIFIER_BIT) != 0U;
}

inline bool CanId::isForceNoFd(uint32_t const value)
{
    return (value & FORCE_NON_FD_QUALIFIER_BIT) != 0U;
}

} /* namespace can */

