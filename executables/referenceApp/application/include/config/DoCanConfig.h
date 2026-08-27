/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/
#pragma once

#include "app/appConfig.h"
#include "can/canframes/CanId.h"
#include "transport/TransportConfiguration.h"

#include <cstdint>

/**
 * Single source of truth for all DoCAN addressing configuration on CAN_0.
 * Re-targeting a deployment (a CAN id, a range, a tester id) is a one-file
 * change.
 *
 * OWNERSHIP:
 *   - CAN identifiers and tester ids below are OWNED here.
 *   - ::can::DoCanMultiAddressingTransportLayer's public static members
 *     (NORMAL_ADDRESSING_TESTER_ID etc.) reference these config:: values
 *     instead of defining their own literals, so every existing call site
 *     that reads MultiLayer::NORMAL_ADDRESSING_TESTER_ID keeps compiling
 *     unchanged, while there is only a single real owner of the value.
 *   - This header deliberately does NOT include
 *     DoCanMultiAddressingTransportLayer.h (that class includes THIS header
 *     instead), so there is no circular include.
 *
 * No addressing logic lives here - values and compile-time checks only.
 */
namespace docan
{
namespace config
{
// ===========================================================================
// Tester addresses (transport ids), one per addressing scheme. OWNED here.
//   Normal (NA)             0xF1
//   Range Extended (REA)    0xF2
//   Normal Fixed (NF)       0xF3
//   Extended (EA)           0xF4
// ===========================================================================
constexpr uint16_t NORMAL_ADDRESSING_TESTER_ID         = 0xF1U;
constexpr uint16_t RANGE_EXTENDED_ADDRESSING_TESTER_ID = 0xF2U;
constexpr uint16_t NORMAL_FIXED_ADDRESSING_TESTER_ID   = 0xF3U;
constexpr uint16_t EXTENDED_ADDRESSING_TESTER_ID       = 0xF4U;

/// UDS functional (broadcast) target address (owned by TransportConfiguration).
constexpr uint16_t FUNCTIONAL_ALL_ISO14229
    = ::transport::TransportConfiguration::FUNCTIONAL_ALL_ISO14229;

/// Standard OBD/UDS functional (group) address legislated by ISO 15765-4 for
/// Normal Fixed Addressing. Unlike the other (table-based) addressing
/// schemes, this wire-level value cannot be freely chosen to already equal
/// FUNCTIONAL_ALL_ISO14229, so incoming functional requests using it need
/// remapping (see DoCanMultiAddressingTransportLayer::
/// NormalFixedFunctionalAddressRemapper).
constexpr uint8_t NORMAL_FIXED_FUNCTIONAL_ADDRESS = 0x33U;

/// ECU / server logical (source) address.
constexpr uint16_t ECU_LOGICAL_ADDRESS = LOGICAL_ADDRESS;

// ===========================================================================
// Normal Addressing (11-bit) - legislative ISO 15765-4 (OBD). OWNED here.
// Physical request/response for the first ECU plus the legislative OBD
// functional (broadcast) request id, which every OBD-compliant ECU must also
// receive on (and reply to physically via the response id below).
// ===========================================================================
constexpr uint32_t NORMAL_ADDRESSING_REQUEST_CAN_ID    = 0x7E0U;
constexpr uint32_t NORMAL_ADDRESSING_RESPONSE_CAN_ID   = 0x7E8U;
constexpr uint32_t NORMAL_ADDRESSING_FUNCTIONAL_CAN_ID = 0x7DFU;

// ===========================================================================
// Extended Addressing (11-bit) - explicit (table-based) ISO 15765-2.
// Fixed single-pair demo defaults, NOT a range/base address - see Range
// Extended Addressing below for the ranged scheme (separate tester ID 0xF2,
// window 0x680-0x77F). Chosen clear of the legislative NA ids above.
// Functional (broadcast) requests use the same request id, with the target
// extension byte set to FUNCTIONAL_ALL_ISO14229 instead of the ECU's own
// address (with EA the CAN id identifies the sender, not phys vs func).
// Demo default - customers/OEMs may reconfigure the base/window as needed.
// ===========================================================================
constexpr uint32_t EXTENDED_ADDRESSING_REQUEST_CAN_ID  = 0x600U;
constexpr uint32_t EXTENDED_ADDRESSING_RESPONSE_CAN_ID = 0x601U;

// ===========================================================================
// Range Extended Addressing (11-bit) - DoCanRangeExtendedAddressingFilter.
// This IS the ranged/base-address scheme (counterpart to the fixed
// single-pair Extended Addressing above) - separate tester ID 0xF2, own
// filter, never overlapping EA's fixed pair or tester ID 0xF4.
// Base CAN id 0x680, arithmetically mapped onto all 256 transport addresses
// 0x00-0xFF, so the range is 0x680-0x77F. Anchored at 0x680 (not 0x700) so it
// ends at 0x77F, staying clear of the legislative NA ids 0x7E0/0x7E8/0x7DF.
// Demo default - customers/OEMs may reconfigure the base/window as needed.
// ===========================================================================
constexpr uint32_t RANGE_EXTENDED_ADDRESSING_BASE_CAN_ID       = 0x680U;
constexpr uint16_t RANGE_EXTENDED_ADDRESSING_BASE_TRANSPORT_ID = 0x000U;
constexpr uint16_t RANGE_EXTENDED_ADDRESSING_COUNT             = 0x100U;
constexpr uint32_t RANGE_EXTENDED_ADDRESSING_LAST_CAN_ID
    = RANGE_EXTENDED_ADDRESSING_BASE_CAN_ID + RANGE_EXTENDED_ADDRESSING_COUNT - 1U;

// ===========================================================================
// Startup validation - guarantee the 11-bit CAN-id windows never overlap.
// These fire at compile time, so a bad re-target cannot ship.
// ===========================================================================
constexpr bool
rangesOverlap(uint32_t const aMin, uint32_t const aMax, uint32_t const bMin, uint32_t const bMax)
{
    return (aMin <= bMax) && (bMin <= aMax);
}

static_assert(RANGE_EXTENDED_ADDRESSING_COUNT > 0U, "Range count must be greater than zero");

static_assert(
    NORMAL_ADDRESSING_TESTER_ID != RANGE_EXTENDED_ADDRESSING_TESTER_ID,
    "Tester IDs must be unique");
static_assert(
    NORMAL_ADDRESSING_TESTER_ID != NORMAL_FIXED_ADDRESSING_TESTER_ID, "Tester IDs must be unique");
static_assert(
    NORMAL_ADDRESSING_TESTER_ID != EXTENDED_ADDRESSING_TESTER_ID, "Tester IDs must be unique");
static_assert(
    RANGE_EXTENDED_ADDRESSING_TESTER_ID != NORMAL_FIXED_ADDRESSING_TESTER_ID,
    "Tester IDs must be unique");
static_assert(
    RANGE_EXTENDED_ADDRESSING_TESTER_ID != EXTENDED_ADDRESSING_TESTER_ID,
    "Tester IDs must be unique");
static_assert(
    NORMAL_FIXED_ADDRESSING_TESTER_ID != EXTENDED_ADDRESSING_TESTER_ID,
    "Tester IDs must be unique");

// Range Extended window must end below the legislative NA ids ...
static_assert(
    RANGE_EXTENDED_ADDRESSING_LAST_CAN_ID < NORMAL_ADDRESSING_FUNCTIONAL_CAN_ID,
    "Range Extended range must end below the legislative NA ids (0x7DF/0x7E0/0x7E8)");
// ... and must not overlap the explicit Extended Addressing ids.
static_assert(
    !rangesOverlap(
        EXTENDED_ADDRESSING_REQUEST_CAN_ID,
        EXTENDED_ADDRESSING_RESPONSE_CAN_ID,
        RANGE_EXTENDED_ADDRESSING_BASE_CAN_ID,
        RANGE_EXTENDED_ADDRESSING_LAST_CAN_ID),
    "Extended and Range Extended CAN-id ranges must be disjoint");

} // namespace config
} // namespace docan
