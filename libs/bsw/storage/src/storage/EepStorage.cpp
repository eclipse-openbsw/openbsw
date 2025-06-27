// Copyright 2025 Accenture.

#include <bsp/eeprom/IEepromDriver.h>
#include <storage/EepStorage.h>
#include <storage/StorageJob.h>
#include <util/crc/Crc.h>

#include <estd/big_endian.h>

#include <cstring>

namespace
{

using NvCrc16 = ::util::crc::CrcRegister<uint16_t, 0x1021U, 0x76B5U>;

bool isCrcValid(uint8_t const* const data, uint16_t const dataSize, uint8_t const* const crc)
{
    NvCrc16 c;
    (void)c.update(data, dataSize);
    return (c.digest() == ::estd::read_be<uint16_t>(crc));
}

void calculateCrc(uint8_t const* const data, uint16_t const dataSize, uint8_t* const crc)
{
    NvCrc16 c;
    (void)c.update(data, dataSize);
    ::estd::write_be<uint16_t>(crc, c.digest());
}

} // anonymous namespace

namespace storage
{

EepStorage::EepStorage(
    EepBlockConfig const* const config,
    size_t const configSize,
    ::eeprom::IEepromDriver& eeprom,
    uint8_t* const eepBuf,
    size_t const eepBufSize,
    size_t const nvCrcSize,
    size_t const sizeTagSize)
: _config(config)
, _configSize(configSize)
, _eeprom(eeprom)
, _eepBuf(eepBuf)
, _eepBufSize(eepBufSize)
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
    auto const totalSize = headerSize + confEntry.blockSize;
    if (totalSize > _eepBufSize)
    {
        job.sendResult(StorageJob::Result::Error());
        return;
    }

    StorageJob::ResultType result = StorageJob::Result::Error();
    if (job.is<StorageJob::Type::Write>())
    {
        result = write(job, confEntry, headerSize, totalSize);
    }
    else if (job.is<StorageJob::Type::Read>())
    {
        result = read(job, confEntry, headerSize, totalSize);
    }
    job.sendResult(result);
}

StorageJob::ResultType EepStorage::write(
    StorageJob& job,
    EepBlockConfig const& confEntry,
    size_t const headerSize,
    size_t const totalSize)
{
    auto& writeJob    = job.getWrite();
    auto const offset = writeJob.getOffset();
    if (offset >= confEntry.blockSize)
    {
        return StorageJob::Result::Error();
    }
    uint16_t usedBlockSize      = 0U;
    // NOTE: the two pointers below are relevant only if errorDetection is enabled
    uint8_t* const blockSizePtr = &(_eepBuf[_nvCrcSize]);
    uint8_t* const dataPtr      = &(_eepBuf[_headerSize]);
    if (confEntry.errorDetection)
    {
        // in case of writing with checksum, read back the previously written data first in order
        // to find out how much data has been written before (if any): this must be taken into
        // account before writing the new size
        if (_eeprom.read(confEntry.address, _eepBuf, totalSize) != ::bsp::BSP_OK)
        {
            return StorageJob::Result::Error();
        }
        usedBlockSize = ::estd::read_be<uint16_t>(blockSizePtr);
        if (usedBlockSize > confEntry.blockSize)
        {
            usedBlockSize = confEntry.blockSize;
        }
        if (!isCrcValid(dataPtr, usedBlockSize, _eepBuf))
        {
            // block is uninitialized or corrupt, use known values for any data before the offset
            (void)memset(dataPtr, 0U, offset);
            // forget the found size as well, since it might be trash
            usedBlockSize = 0U;
        }
    }
    auto progressInBlock   = offset;
    size_t progressForUser = 0U;
    for (auto const& writeSlice : writeJob.getBuffer())
    {
        auto sizeToCopy = writeSlice.size();
        if ((progressInBlock + sizeToCopy) > confEntry.blockSize)
        {
            // trying to store too much data
            return StorageJob::Result::Error();
        }
        (void)memcpy(&(_eepBuf[headerSize + progressInBlock]), writeSlice.data(), sizeToCopy);
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
        if (progressInBlock > usedBlockSize)
        {
            // update size if more will be written than was there before
            usedBlockSize = progressInBlock;
        }
        ::estd::write_be<uint16_t>(blockSizePtr, usedBlockSize);
        calculateCrc(dataPtr, usedBlockSize, _eepBuf);
    }
    if (_eeprom.write(confEntry.address, _eepBuf, headerSize + progressInBlock) != ::bsp::BSP_OK)
    {
        return StorageJob::Result::Error();
    }
    return StorageJob::Result::Success();
}

StorageJob::ResultType EepStorage::read(
    StorageJob& job,
    EepBlockConfig const& confEntry,
    size_t const headerSize,
    size_t const totalSize)
{
    // not strictly required, but reset the internal buffer first just to be more deterministic
    (void)memset(_eepBuf, 0U, totalSize);
    // read the complete block in one go
    if (_eeprom.read(confEntry.address, _eepBuf, totalSize) != ::bsp::BSP_OK)
    {
        return StorageJob::Result::Error();
    }
    // for blocks without error detection, consider max capacity as the "used block size"
    auto usedBlockSize = confEntry.blockSize;
    if (confEntry.errorDetection)
    {
        // we need to find the previously stored size to verify the checksum and to know how much
        // can be copied to the read buffer
        usedBlockSize = ::estd::read_be<uint16_t>(&(_eepBuf[_nvCrcSize]));
        if (usedBlockSize > confEntry.blockSize)
        {
            // limit used block size to the max capacity
            // NOTE: if more data was stored using another SW where the block size was bigger, it
            // will fail the checksum check and get discarded
            usedBlockSize = confEntry.blockSize;
        }
        if (!isCrcValid(&(_eepBuf[headerSize]), usedBlockSize, _eepBuf))
        {
            return StorageJob::Result::DataLoss();
        }
    }
    auto& readJob          = job.getRead();
    auto progressInBlock   = readJob.getOffset();
    size_t progressForUser = 0U;
    // copy data into the provided buffers one after another from continuous locations,
    // incrementing the location by the buffer size after each copy
    for (auto& readSlice : readJob.getBuffer())
    {
        if (progressInBlock >= usedBlockSize)
        {
            break;
        }
        auto sizeToCopy = readSlice.size();
        if ((progressInBlock + sizeToCopy) > usedBlockSize)
        {
            // limit the copy size to as much as is available in EEPROM
            // NOTE: progressInBlock must never become bigger than usedBlockSize (that's why it's
            // checked in the beginning of the loop), otherwise sizeToCopy might overflow
            sizeToCopy = usedBlockSize - progressInBlock;
        }
        (void)memcpy(readSlice.data(), &(_eepBuf[headerSize + progressInBlock]), sizeToCopy);
        progressForUser += sizeToCopy;
        progressInBlock += sizeToCopy;
    }
    // report how many bytes were read
    readJob.setReadSize(progressForUser);
    return StorageJob::Result::Success();
}

} // namespace storage
