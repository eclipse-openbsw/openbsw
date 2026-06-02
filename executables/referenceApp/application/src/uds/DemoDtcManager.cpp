/********************************************************************************
 * Copyright (c) 2026 An Dao
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "uds/DemoDtcManager.h"

namespace uds
{

DemoDtcManager::DemoDtcManager() : _dtcs{}, _dtcCount(0U), _dtcSettingEnabled(true) {}

void DemoDtcManager::reportFault(uint32_t const dtcNumber)
{
    if (!_dtcSettingEnabled)
    {
        return;
    }

    DtcEntry* entry = findOrCreate(dtcNumber);
    if (entry != nullptr)
    {
        entry->statusByte |= STATUS_TEST_FAILED | STATUS_CONFIRMED;
        entry->statusByte &= static_cast<uint8_t>(~STATUS_TEST_NOT_COMPLETE);
    }
}

void DemoDtcManager::clearAll()
{
    for (size_t i = 0U; i < _dtcCount; ++i)
    {
        _dtcs[i].statusByte = 0U;
        _dtcs[i].active     = false;
    }
    _dtcCount = 0U;
}

void DemoDtcManager::clearByGroup(uint32_t const groupOfDtc)
{
    if (groupOfDtc == 0xFFFFFFU)
    {
        clearAll();
        return;
    }

    for (size_t i = 0U; i < _dtcCount; ++i)
    {
        if (_dtcs[i].active && (_dtcs[i].dtcNumber == groupOfDtc))
        {
            _dtcs[i].statusByte = 0U;
            _dtcs[i].active     = false;
        }
    }
}

uint16_t DemoDtcManager::getCountByStatusMask(uint8_t const statusMask) const
{
    uint16_t count = 0U;
    for (size_t i = 0U; i < _dtcCount; ++i)
    {
        if (_dtcs[i].active && ((_dtcs[i].statusByte & statusMask) != 0U))
        {
            ++count;
        }
    }
    return count;
}

size_t DemoDtcManager::getDtcsByStatusMask(
    uint8_t const statusMask, ::etl::span<uint8_t> const buffer) const
{
    return collectDtcs(buffer, true, statusMask);
}

size_t DemoDtcManager::getSupportedDtcs(::etl::span<uint8_t> const buffer) const
{
    return collectDtcs(buffer, false, 0U);
}

size_t DemoDtcManager::collectDtcs(
    ::etl::span<uint8_t> const buffer,
    bool const filterByStatusMask,
    uint8_t const statusMask) const
{
    size_t count  = 0U;
    size_t offset = 0U;

    for (size_t i = 0U; i < _dtcCount; ++i)
    {
        if (!_dtcs[i].active)
        {
            continue;
        }
        if (filterByStatusMask && ((_dtcs[i].statusByte & statusMask) == 0U))
        {
            continue;
        }
        if ((offset + 4U) > buffer.size())
        {
            break;
        }
        // DTC high, mid, low bytes + status byte
        buffer[offset]     = static_cast<uint8_t>((_dtcs[i].dtcNumber >> 16U) & 0xFFU);
        buffer[offset + 1] = static_cast<uint8_t>((_dtcs[i].dtcNumber >> 8U) & 0xFFU);
        buffer[offset + 2] = static_cast<uint8_t>(_dtcs[i].dtcNumber & 0xFFU);
        buffer[offset + 3] = _dtcs[i].statusByte;
        offset += 4U;
        ++count;
    }
    return count;
}

DemoDtcManager::DtcEntry* DemoDtcManager::findOrCreate(uint32_t const dtcNumber)
{
    DtcEntry* freeSlot = nullptr;
    for (size_t i = 0U; i < _dtcCount; ++i)
    {
        if (_dtcs[i].active && (_dtcs[i].dtcNumber == dtcNumber))
        {
            return &_dtcs[i];
        }
        // Reuse cleared (inactive) slots so fault/clear cycles don't exhaust the array.
        if ((!_dtcs[i].active) && (freeSlot == nullptr))
        {
            freeSlot = &_dtcs[i];
        }
    }

    if ((freeSlot == nullptr) && (_dtcCount < MAX_DTCS))
    {
        freeSlot = &_dtcs[_dtcCount];
        ++_dtcCount;
    }

    if (freeSlot != nullptr)
    {
        freeSlot->dtcNumber  = dtcNumber;
        freeSlot->statusByte = 0U;
        freeSlot->active     = true;
    }

    return freeSlot;
}

} // namespace uds
