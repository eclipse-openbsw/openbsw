// Copyright 2026 KPIT Technologies
// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <cstdint>

namespace uds
{
struct DtcRecord;
struct FreezeFrameRecord;

/**
 * Abstract NVM storage interface for DTC records (DGN-FEL009/010).
 * Decouples FaultManager from physical NVM driver.
 * The primary record occupies slot [index]; the redundant backup copy
 * occupies slot [index + MAX_DTC_RECORDS] (DGN-FEL010).
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
