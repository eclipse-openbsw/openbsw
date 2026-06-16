/********************************************************************************
 * Copyright (c) 2026 An Dao
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "platform/estdint.h"

#include <etl/array.h>

namespace uds
{
/**
 * Minimal DTC manager for the STM32 diagnostic demo.
 * In-memory only (no NvM persistence).
 */
class Stm32DtcManager
{
public:
    static uint8_t const MAX_DTCS = 16U;

    // ISO 14229-1 DTC status byte bits
    static uint8_t const STATUS_TEST_FAILED       = 0x01U;
    static uint8_t const STATUS_CONFIRMED         = 0x08U;
    static uint8_t const STATUS_TEST_NOT_COMPLETE = 0x10U;

    struct DtcEntry
    {
        uint32_t dtcNumber; // 3-byte DTC (e.g. 0x123456)
        uint8_t statusByte;
        bool active;
    };

    Stm32DtcManager();

    void reportFault(uint32_t dtcNumber);
    void clearAll();
    void clearByGroup(uint32_t groupOfDtc);

    uint16_t getCountByStatusMask(uint8_t statusMask) const;
    uint8_t getDtcsByStatusMask(uint8_t statusMask, uint8_t* buffer, uint16_t bufferSize) const;
    uint8_t getSupportedDtcs(uint8_t* buffer, uint16_t bufferSize) const;

    void setDtcSettingEnabled(bool enabled) { _dtcSettingEnabled = enabled; }

    bool isDtcSettingEnabled() const { return _dtcSettingEnabled; }

private:
    DtcEntry* findOrCreate(uint32_t dtcNumber);

    etl::array<DtcEntry, MAX_DTCS> _dtcs;
    uint8_t _dtcCount;
    bool _dtcSettingEnabled;
};

} // namespace uds
