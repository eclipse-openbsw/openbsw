// Copyright 2025 Accenture.

#pragma once

#include <storage/IStorage.h>
#include <storage/StorageJob.h>

namespace eeprom
{
class IEepromDriver;
};

namespace storage
{

struct EepBlockConfig
{
    uint32_t const address;
    uint16_t const blockSize;
    bool const errorDetection;
};

class EepStorage : public IStorage
{
public:
    ~EepStorage()                            = default;
    EepStorage(EepStorage const&)            = delete;
    EepStorage& operator=(EepStorage const&) = delete;

    void process(StorageJob& job) final;

protected:
    explicit EepStorage(
        EepBlockConfig const* config,
        size_t configSize,
        ::eeprom::IEepromDriver& eeprom,
        uint8_t* eepBuf,
        size_t eepBufSize,
        size_t nvCrcSize,
        size_t sizeTagSize);

private:
    StorageJob::ResultType
    write(StorageJob& job, EepBlockConfig const& conf, size_t headerSize, size_t totalSize);
    StorageJob::ResultType
    read(StorageJob& job, EepBlockConfig const& conf, size_t headerSize, size_t totalSize);

    EepBlockConfig const* const _config;
    size_t const _configSize;
    ::eeprom::IEepromDriver& _eeprom;
    uint8_t* const _eepBuf;
    size_t const _eepBufSize;
    size_t const _nvCrcSize;
    size_t const _headerSize;
};

namespace declare
{
// CONFIG_SIZE: number of entries in the config
// MAX_BLOCK_SIZE: maximum block size (without the header) present in the config
template<size_t CONFIG_SIZE, size_t MAX_BLOCK_SIZE>
class EepStorage : public ::storage::EepStorage
{
    static_assert(CONFIG_SIZE > 0U, "number of blocks must be higher than 0");
    static_assert(MAX_BLOCK_SIZE > 0U, "maximum block size must be higher than 0");

public:
    static constexpr size_t NV_CRC_SIZE  = 2U;
    static constexpr size_t SIZETAG_SIZE = 2U; // size of the stored block size in bytes
    static constexpr size_t HEADER_SIZE  = NV_CRC_SIZE + SIZETAG_SIZE;
    static constexpr size_t BUFFER_SIZE  = HEADER_SIZE + MAX_BLOCK_SIZE;

    explicit EepStorage(
        EepBlockConfig const (&config)[CONFIG_SIZE], ::eeprom::IEepromDriver& eeprom)
    : ::storage::EepStorage(
        reinterpret_cast<EepBlockConfig const*>(&config),
        CONFIG_SIZE,
        eeprom,
        _eepBuf,
        BUFFER_SIZE,
        NV_CRC_SIZE,
        SIZETAG_SIZE)
    {}

private:
    uint8_t _eepBuf[HEADER_SIZE + MAX_BLOCK_SIZE];
};
} // namespace declare

} // namespace storage
