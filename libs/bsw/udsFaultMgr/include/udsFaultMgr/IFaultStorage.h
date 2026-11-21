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
struct DtcRecord;
struct FreezeFrameRecord;

/**
 * Abstract  storage interface for DTC records
 * Decouples FaultManager from physical storage driver.
 * The primary record occupies slot [index]; the redundant backup copy
 * occupies slot [index + MAX_DTC_RECORDS]
 */
class IFaultStorage
{
public:
    virtual ~IFaultStorage() = default;

    virtual bool readDtcRecord(uint8_t index, DtcRecord& out)                     = 0;
    virtual bool writeDtcRecord(uint8_t index, DtcRecord const& record)           = 0;
    virtual bool readFreezeFrame(uint8_t index, FreezeFrameRecord& out)           = 0;
    virtual bool writeFreezeFrame(uint8_t index, FreezeFrameRecord const& record) = 0;
    virtual bool eraseAll()                                                       = 0;
};

} // namespace uds
