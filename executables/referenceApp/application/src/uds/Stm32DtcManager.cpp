// Copyright 2025 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

#include "uds/Stm32DtcManager.h"

#include <cstring>

namespace uds
{

Stm32DtcManager::Stm32DtcManager() : _dtcs{}, _dtcCount(0U), _dtcSettingEnabled(true) {}

void Stm32DtcManager::reportFault(uint32_t const dtcNumber)
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

void Stm32DtcManager::clearAll()
{
    for (uint8_t i = 0U; i < _dtcCount; ++i)
    {
        _dtcs[i].statusByte = 0U;
        _dtcs[i].active     = false;
    }
    _dtcCount = 0U;
}

void Stm32DtcManager::clearByGroup(uint32_t const groupOfDtc)
{
    if (groupOfDtc == 0xFFFFFFU)
    {
        clearAll();
        return;
    }

    for (uint8_t i = 0U; i < _dtcCount; ++i)
    {
        if (_dtcs[i].active && (_dtcs[i].dtcNumber == groupOfDtc))
        {
            _dtcs[i].statusByte = 0U;
            _dtcs[i].active     = false;
        }
    }
}

uint16_t Stm32DtcManager::getCountByStatusMask(uint8_t const statusMask) const
{
    uint16_t count = 0U;
    for (uint8_t i = 0U; i < _dtcCount; ++i)
    {
        if (_dtcs[i].active && ((_dtcs[i].statusByte & statusMask) != 0U))
        {
            ++count;
        }
    }
    return count;
}

uint8_t Stm32DtcManager::getDtcsByStatusMask(
    uint8_t const statusMask, uint8_t* const buffer, uint16_t const bufferSize) const
{
    uint8_t count  = 0U;
    uint16_t offset = 0U;

    for (uint8_t i = 0U; i < _dtcCount; ++i)
    {
        if (_dtcs[i].active && ((_dtcs[i].statusByte & statusMask) != 0U))
        {
            if ((offset + 4U) > bufferSize)
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
    }
    return count;
}

uint8_t Stm32DtcManager::getSupportedDtcs(uint8_t* const buffer, uint16_t const bufferSize) const
{
    uint8_t count  = 0U;
    uint16_t offset = 0U;

    for (uint8_t i = 0U; i < _dtcCount; ++i)
    {
        if (_dtcs[i].active)
        {
            if ((offset + 4U) > bufferSize)
            {
                break;
            }
            buffer[offset]     = static_cast<uint8_t>((_dtcs[i].dtcNumber >> 16U) & 0xFFU);
            buffer[offset + 1] = static_cast<uint8_t>((_dtcs[i].dtcNumber >> 8U) & 0xFFU);
            buffer[offset + 2] = static_cast<uint8_t>(_dtcs[i].dtcNumber & 0xFFU);
            buffer[offset + 3] = _dtcs[i].statusByte;
            offset += 4U;
            ++count;
        }
    }
    return count;
}

Stm32DtcManager::DtcEntry* Stm32DtcManager::findOrCreate(uint32_t const dtcNumber)
{
    for (uint8_t i = 0U; i < _dtcCount; ++i)
    {
        if (_dtcs[i].active && (_dtcs[i].dtcNumber == dtcNumber))
        {
            return &_dtcs[i];
        }
    }

    if (_dtcCount < MAX_DTCS)
    {
        DtcEntry& entry = _dtcs[_dtcCount];
        entry.dtcNumber = dtcNumber;
        entry.statusByte = 0U;
        entry.active     = true;
        ++_dtcCount;
        return &entry;
    }

    return nullptr;
}

Stm32DtcManager::DtcEntry const* Stm32DtcManager::find(uint32_t const dtcNumber) const
{
    for (uint8_t i = 0U; i < _dtcCount; ++i)
    {
        if (_dtcs[i].active && (_dtcs[i].dtcNumber == dtcNumber))
        {
            return &_dtcs[i];
        }
    }
    return nullptr;
}

} // namespace uds
