/********************************************************************************
 * Copyright (c) 2026 KPIT Technologies Ltd
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/
#pragma once
#include <cstdint>

namespace uds
{

class FaultManager;

static constexpr uint8_t MAX_INHIBITING_DTCS_PER_FUNCTION = 4U;
static constexpr uint8_t MAX_INHIBITION_ENTRIES           = 32U;

/**
 * Compile/init-time entry mapping a software function to the DTCs that inhibit it.
 *
 */
struct InhibitionEntry
{
    uint16_t functionId;                                         ///< Software function ID
    uint32_t inhibitingDtcIds[MAX_INHIBITING_DTCS_PER_FUNCTION]; ///< Inhibiting DTC IDs
    uint8_t inhibitingDtcCount;                                  ///< Valid entries in array
};

/**
 * InhibitionTable: feature inhibition logic
 * Checks whether a software function is allowed to execute based on
 * the confirmedDTC status of its dependent DTCs.
 */
class InhibitionTable
{
public:
    InhibitionTable();

    void init(FaultManager& faultManager);

    /**
     * Register a function-to-DTC inhibition mapping.
     * \return true if registered; false if table full or functionId duplicate
     */
    bool registerEntry(InhibitionEntry const& entry);

    /**
     * Query whether a function is allowed to execute.
     * Returns false if any inhibiting DTC is in confirmedDTC state.
     * Returns true by default for unregistered functionIds
     */
    bool isAllowed(uint16_t functionId) const;

private:
    InhibitionEntry _entries[MAX_INHIBITION_ENTRIES];
    uint8_t _entryCount;
    FaultManager* _faultManager;
};

} // namespace uds
