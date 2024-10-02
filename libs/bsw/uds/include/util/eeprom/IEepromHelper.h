// Copyright 2024 Accenture.

#ifndef GUARD_9CA90E28_3D12_4C5B_9EAA_FF2CDB970ABF
#define GUARD_9CA90E28_3D12_4C5B_9EAA_FF2CDB970ABF

#include "nvstorage/NvStorageTypes.h"
#include "platform/estdint.h"

#include <cstddef>

namespace eeprom
{
using EepromJobFinishedNotification = ::nvstorage::applJobFinishedCallback;

class IEepromHelper
{
public:
    virtual bool read(
        ::nvstorage::NvBlockIdType blockId,
        uint8_t* buffer,
        size_t length,
        EepromJobFinishedNotification callback)
        = 0;

    virtual bool write(
        ::nvstorage::NvBlockIdType blockId,
        uint8_t const* buffer,
        size_t length,
        EepromJobFinishedNotification callback)
        = 0;
};

} // namespace eeprom

#endif // GUARD_9CA90E28_3D12_4C5B_9EAA_FF2CDB970ABF
