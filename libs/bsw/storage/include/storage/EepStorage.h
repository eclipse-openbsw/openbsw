// Copyright 2025 Accenture.

#pragma once

#include <etl/array.h>
#include <etl/span.h>
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
    uint16_t const dataSize;
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
        ::etl::span<uint8_t> eepBuf,
        size_t nvCrcSize,
        size_t sizeTagSize);

private:
    StorageJob::ResultType write(
        StorageJob& job,
        EepBlockConfig const& conf,
        size_t headerSize,
        ::etl::span<uint8_t> writeBuf);
    StorageJob::ResultType read(
        StorageJob& job,
        EepBlockConfig const& conf,
        size_t headerSize,
        ::etl::span<uint8_t> readBuf);

    EepBlockConfig const* const _config;
    size_t const _configSize;
    ::eeprom::IEepromDriver& _eeprom;
    ::etl::span<uint8_t> const _eepBuf;
    size_t const _nvCrcSize;
    size_t const _headerSize;
};

namespace declare
{
// CONFIG_SIZE: number of entries in the config
// MAX_DATA_SIZE: maximum data size present in the config
template<size_t CONFIG_SIZE, size_t MAX_DATA_SIZE>
class EepStorage : public ::storage::EepStorage
{
    static_assert(CONFIG_SIZE > 0U, "number of blocks must be bigger than 0");
    static_assert(MAX_DATA_SIZE > 0U, "maximum data size must be bigger than 0");

public:
    static constexpr size_t CRC_SIZE     = 2U;
    static constexpr size_t SIZETAG_SIZE = 2U; // size of the stored data size in bytes

    explicit EepStorage(
        EepBlockConfig const (&config)[CONFIG_SIZE], ::eeprom::IEepromDriver& eeprom)
    : ::storage::EepStorage(
        reinterpret_cast<EepBlockConfig const*>(&config),
        CONFIG_SIZE,
        eeprom,
        _eepBuf,
        CRC_SIZE,
        SIZETAG_SIZE)
    {}

private:
    ::etl::array<uint8_t, CRC_SIZE + SIZETAG_SIZE + MAX_DATA_SIZE> _eepBuf;
};
} // namespace declare

} // namespace storage
