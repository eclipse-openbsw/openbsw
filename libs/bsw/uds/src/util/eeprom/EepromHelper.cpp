// Copyright 2024 Accenture.

#include "util/eeprom/EepromHelper.h"

#include "nvstorage/INvStorage.h"
#include "uds/UdsLogger.h"

#include <etl/span.h>

namespace eeprom
{
using ::nvstorage::INvStorage;
using ::util::logger::Logger;
using ::util::logger::UDS;

EepromHelper::EepromHelper(::nvstorage::INvStorage& invstorage, ::async::ContextType const context)
: _iNvstorage(invstorage)
, _context(context)
, _eepromJobFinishedCallback()
, _executeJobfinished()
, _nvStorageOperation(nvstorage::NVSTORAGE_NONE)
, _nvStorageReturnCode(nvstorage::NVSTORAGE_REQ_OK)
{}

bool EepromHelper::read(
    ::nvstorage::NvBlockIdType blockId,
    uint8_t* buffer,
    size_t length,
    EepromJobFinishedNotification callback)
{
    _eepromJobFinishedCallback = callback;
    auto const readData        = ::etl::span<uint8_t>(buffer, length);

    auto const jobfinished = EepromJobFinishedNotification::
        create<EepromHelper, &EepromHelper::nvStorageFinishedCallback>(*this);
    auto const readJobResult = _iNvstorage.blockRead(blockId, readData, jobfinished, {});
    if ((readJobResult != ::nvstorage::NVSTORAGE_REQ_OK)
        && (readJobResult != ::nvstorage::NVSTORAGE_REQ_PENDING))
    {
        Logger::warn(UDS, "EepromHelper::read() Error reading data");
        return false;
    }
    return true;
}

bool EepromHelper::write(
    ::nvstorage::NvBlockIdType blockId,
    uint8_t const* buffer,
    size_t length,
    EepromJobFinishedNotification callback)
{
    _eepromJobFinishedCallback = callback;
    auto const writeData       = ::etl::span<uint8_t>(const_cast<uint8_t*>(buffer), length);

    auto const jobfinished = EepromJobFinishedNotification::
        create<EepromHelper, &EepromHelper::nvStorageFinishedCallback>(*this);

    auto const writeJobResult = _iNvstorage.blockWrite(
        blockId, writeData, nvstorage::NVSTORAGE_PRIORITY_LOW, jobfinished);
    if ((writeJobResult != ::nvstorage::NVSTORAGE_REQ_OK)
        && (writeJobResult != ::nvstorage::NVSTORAGE_REQ_PENDING))
    {
        Logger::warn(UDS, "EepromHelper::write() Error writing data");
        return false;
    }
    return true;
}

void EepromHelper::jobFinishedCallback()
{
    _eepromJobFinishedCallback(_nvStorageOperation, _nvStorageReturnCode, _nvBlockIdType);
}

void EepromHelper::nvStorageFinishedCallback(
    ::nvstorage::NvStorageOperation const op,
    ::nvstorage::NvStorageReturnCode const res,
    ::nvstorage::NvBlockIdType const id)
{
    _nvStorageOperation  = op;
    _nvStorageReturnCode = res;
    _nvBlockIdType       = id;

    _executeJobfinished
        = ::async::Function::CallType::create<EepromHelper, &EepromHelper::jobFinishedCallback>(
            *this);
    ::async::execute(_context, _executeJobfinished);
}

} // namespace eeprom
