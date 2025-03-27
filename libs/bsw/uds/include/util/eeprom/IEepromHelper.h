// Copyright 2024 Accenture.

#pragma once

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
