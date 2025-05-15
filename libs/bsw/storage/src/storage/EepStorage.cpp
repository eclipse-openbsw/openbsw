// Copyright 2025 Accenture.

#include <bsp/eeprom/IEepromDriver.h>
#include <etl/crc16_aug_ccitt.h>
#include <etl/memory.h>
#include <etl/unaligned_type.h>
#include <storage/EepStorage.h>
#include <storage/StorageJob.h>

namespace
{

// NOTE: using CRC type AUG-CCITT instead of, say, CCITT mainly because the former gives
// "nontrivial" checksums (i.e. not 0 or 0xFF) for the most common input data 0 or 0xFF
using CrcType = ::etl::crc16_aug_ccitt_t256;

bool isCrcValid(::etl::span<uint8_t const> const data, uint8_t const* const crc)
{
    CrcType c;
    c.add(data.begin(), data.end());
    return (c.value() == ::etl::be_uint16_t{crc});
}

void calculateCrc(::etl::span<uint8_t const> const data, uint8_t* const crc)
{
    CrcType c;
    c.add(data.begin(), data.end());
    ::etl::be_uint16_ext_t{crc} = c.value();
}

} // anonymous namespace

namespace storage
{

EepStorage::EepStorage(
    EepBlockConfig const* const config,
    size_t const configSize,
    ::eeprom::IEepromDriver& eeprom,
    ::etl::span<uint8_t> const eepBuf,
    size_t const nvCrcSize,
    size_t const sizeTagSize)
: _config(config)
, _configSize(configSize)
, _eeprom(eeprom)
, _eepBuf(eepBuf)
, _nvCrcSize(nvCrcSize)
, _headerSize(_nvCrcSize + sizeTagSize)
{}

void EepStorage::process(StorageJob& job)
{
    if (job.getId() >= _configSize)
    {
        job.sendResult(StorageJob::Result::Error());
        return;
    }

    auto const& confEntry = _config[job.getId()];
    size_t headerSize     = 0U;
    if (confEntry.errorDetection)
    {
        headerSize = _headerSize;
    }
    auto const totalSize = headerSize + confEntry.dataSize;
    if (totalSize > _eepBuf.size())
    {
        job.sendResult(StorageJob::Result::Error());
        return;
    }

    auto const buf                = ::etl::span<uint8_t>(_eepBuf).first(totalSize);
    StorageJob::ResultType result = StorageJob::Result::Error();
    if (job.is<StorageJob::Type::Write>())
    {
        result = write(job, confEntry, headerSize, buf);
    }
    else if (job.is<StorageJob::Type::Read>())
    {
        result = read(job, confEntry, headerSize, buf);
    }
    job.sendResult(result);
}

StorageJob::ResultType EepStorage::write(
    StorageJob& job,
    EepBlockConfig const& confEntry,
    size_t const headerSize,
    ::etl::span<uint8_t> const eepBuf)
{
    auto& writeJob    = job.getWrite();
    auto const offset = writeJob.getOffset();
    if (offset >= confEntry.dataSize)
    {
        return StorageJob::Result::Error();
    }
    uint16_t usedDataSize   = 0U;
    // NOTE: the two pointers below are relevant only if errorDetection is enabled
    auto* const dataSizePtr = eepBuf.data() + _nvCrcSize;
    auto* const dataPtr     = eepBuf.data() + _headerSize;
    if (confEntry.errorDetection)
    {
        // in case of writing with checksum, read back the previously written data first in order
        // to find out how much data has been written before (if any): this must be taken into
        // account before writing the new size
        if (_eeprom.read(confEntry.address, eepBuf.data(), eepBuf.size()) != ::bsp::BSP_OK)
        {
            return StorageJob::Result::Error();
        }
        usedDataSize = ::etl::be_uint16_t{dataSizePtr};
        if (usedDataSize > confEntry.dataSize)
        {
            usedDataSize = confEntry.dataSize;
        }
        if (!isCrcValid(::etl::span<uint8_t const>(dataPtr, usedDataSize), eepBuf.data()))
        {
            // block is uninitialized or corrupt, use known values for any data before the offset
            (void)::etl::mem_set(dataPtr, offset, static_cast<uint8_t>(0U));
            // forget the found size as well, since it might be trash
            usedDataSize = 0U;
        }
    }
    auto progressInBlock   = offset;
    size_t progressForUser = 0U;
    for (auto const& writeBuf : writeJob.getBuffer())
    {
        auto sizeToCopy = writeBuf.size();
        if ((progressInBlock + sizeToCopy) > confEntry.dataSize)
        {
            // trying to store too much data
            return StorageJob::Result::Error();
        }
        (void)::etl::mem_copy(
            writeBuf.data(), sizeToCopy, (eepBuf.data() + headerSize + progressInBlock));
        progressForUser += sizeToCopy;
        progressInBlock += sizeToCopy;
    }
    if (progressForUser == 0U)
    {
        // writing zero bytes not allowed
        return StorageJob::Result::Error();
    }
    if (confEntry.errorDetection)
    {
        // store how many bytes were written and the new checksum
        // NOTE: both need to take into account any data before the offset (either previously
        // written or newly initialized), and also any data that was already stored after the last
        // writing address
        if (progressInBlock > usedDataSize)
        {
            // update size if more will be written than was there before
            usedDataSize = progressInBlock;
        }
        ::etl::be_uint16_ext_t{dataSizePtr} = usedDataSize;
        calculateCrc(::etl::span<uint8_t const>(dataPtr, usedDataSize), eepBuf.data());
    }
    if (_eeprom.write(confEntry.address, eepBuf.data(), headerSize + progressInBlock)
        != ::bsp::BSP_OK)
    {
        return StorageJob::Result::Error();
    }
    return StorageJob::Result::Success();
}

StorageJob::ResultType EepStorage::read(
    StorageJob& job,
    EepBlockConfig const& confEntry,
    size_t const headerSize,
    ::etl::span<uint8_t> const eepBuf)
{
    // not strictly required, but reset the internal buffer first just to be more deterministic
    (void)::etl::mem_set(eepBuf.data(), eepBuf.size(), static_cast<uint8_t>(0U));
    // read the complete block in one go
    if (_eeprom.read(confEntry.address, eepBuf.data(), eepBuf.size()) != ::bsp::BSP_OK)
    {
        return StorageJob::Result::Error();
    }
    // for blocks without error detection, consider max capacity as the "used data size"
    auto usedDataSize = confEntry.dataSize;
    if (confEntry.errorDetection)
    {
        // we need to find the previously stored size to verify the checksum and to know how much
        // can be copied to the read buffer
        usedDataSize = ::etl::be_uint16_t{eepBuf.data() + _nvCrcSize};
        if (usedDataSize > confEntry.dataSize)
        {
            // limit used data size to the max capacity
            // NOTE: if more data was stored using another SW where the data size was bigger, it
            // will fail the checksum check and get discarded
            usedDataSize = confEntry.dataSize;
        }
        if (!isCrcValid(
                ::etl::span<uint8_t const>(eepBuf.data() + headerSize, usedDataSize),
                eepBuf.data()))
        {
            return StorageJob::Result::DataLoss();
        }
    }
    auto& readJob          = job.getRead();
    auto progressInBlock   = readJob.getOffset();
    size_t progressForUser = 0U;
    // copy data into the provided buffers one after another from continuous locations,
    // incrementing the location by the buffer size after each copy
    for (auto& readBuf : readJob.getBuffer())
    {
        if (progressInBlock >= usedDataSize)
        {
            break;
        }
        auto sizeToCopy = readBuf.size();
        if ((progressInBlock + sizeToCopy) > usedDataSize)
        {
            // limit the copy size to as much as is available in EEPROM
            // NOTE: progressInBlock must never become bigger than usedDataSize (that's why it's
            // checked in the beginning of the loop), otherwise sizeToCopy might overflow
            sizeToCopy = usedDataSize - progressInBlock;
        }
        (void)::etl::mem_copy(
            (eepBuf.data() + headerSize + progressInBlock), sizeToCopy, readBuf.data());
        progressForUser += sizeToCopy;
        progressInBlock += sizeToCopy;
    }
    // report how many bytes were read
    readJob.setReadSize(progressForUser);
    return StorageJob::Result::Success();
}

} // namespace storage
