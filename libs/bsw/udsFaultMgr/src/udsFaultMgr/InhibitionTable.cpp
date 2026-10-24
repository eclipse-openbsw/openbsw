/********************************************************************************
 * Copyright (c) 2026 KPIT Technologies Ltd
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/
#include "udsFaultMgr/InhibitionTable.h"
#include "udsFaultMgr/DtcRecord.h"
#include "udsFaultMgr/FaultManager.h"

namespace uds
{

InhibitionTable::InhibitionTable() : _entryCount(0U), _faultManager(nullptr) {}

void InhibitionTable::init(FaultManager& faultManager) { _faultManager = &faultManager; }

bool InhibitionTable::registerEntry(InhibitionEntry const& entry)
{
    if (_entryCount >= MAX_INHIBITION_ENTRIES)
    {
        return false;
    }
    // Prevent duplicate functionId
    for (uint8_t i = 0U; i < _entryCount; ++i)
    {
        if (_entries[i].functionId == entry.functionId)
        {
            return false;
        }
    }
    _entries[_entryCount] = entry;
    ++_entryCount;
    return true;
}

bool InhibitionTable::isAllowed(uint16_t functionId) const
{
    if (_faultManager == nullptr)
    {
        return true; // Safe default: allow when not wired
    }
    for (uint8_t i = 0U; i < _entryCount; ++i)
    {
        if (_entries[i].functionId == functionId)
        {
            InhibitionEntry const& e = _entries[i];
            for (uint8_t j = 0U; j < e.inhibitingDtcCount; ++j)
            {
                uint8_t const status = _faultManager->getDtcStatus(e.inhibitingDtcIds[j]);
                if ((status & DtcStatusBits::CONFIRMED_DTC) != 0U)
                {
                    return false;
                }
            }
            return true; // Entry found; no inhibiting DTC is confirmed
        }
    }
    return true; // Not registered — allow by default
}

} // namespace uds
