// Copyright 2024 Accenture.

#pragma once

#include "util/eeprom/IEepromHelper.h"

#include <async/Types.h>
#include <async/util/Call.h>

namespace nvstorage
{
class INvStorage;
}

namespace eeprom
{
class EepromHelper : public IEepromHelper
{
public:
    explicit EepromHelper(::nvstorage::INvStorage& invstorage, ::async::ContextType context);

    virtual bool read(
        ::nvstorage::NvBlockIdType blockId,
        uint8_t* buffer,
        size_t length,
        EepromJobFinishedNotification callback) override;

    virtual bool write(
        ::nvstorage::NvBlockIdType blockId,
        uint8_t const* buffer,
        size_t length,
        EepromJobFinishedNotification callback) override;

private:
    void nvStorageFinishedCallback(
        ::nvstorage::NvStorageOperation const op,
        ::nvstorage::NvStorageReturnCode const res,
        ::nvstorage::NvBlockIdType const id);

    void jobFinishedCallback();

    ::nvstorage::INvStorage& _iNvstorage;
    ::async::ContextType _context;

    EepromJobFinishedNotification _eepromJobFinishedCallback;
    ::async::Function _executeJobfinished;

    ::nvstorage::NvStorageOperation _nvStorageOperation;
    ::nvstorage::NvStorageReturnCode _nvStorageReturnCode;
    ::nvstorage::NvBlockIdType _nvBlockIdType{};
};

} // namespace eeprom

