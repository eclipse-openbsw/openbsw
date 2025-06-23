// Copyright 2025 Accenture.

#pragma once

#include <async/Async.h>
#include <bsp/eeprom/IEepromDriver.h>
#include <console/AsyncCommandWrapper.h>
#include <lifecycle/AsyncLifecycleComponent.h>
#include <storage/EepStorage.h>
#include <storage/FeeStorage.h>
#include <storage/MappingStorage.h>
#include <storage/QueuingStorage.h>
#include <storage/StorageJob.h>
#include <storage/StorageTester.h>

namespace systems
{

// clang-format off
// BEGIN config
static constexpr uint8_t EEP_STORAGE_ID = 0;
static constexpr uint8_t FEE_STORAGE_ID = 1;

static constexpr ::storage::MappingConfig MAPPING_CONFIG[] = {
    {
        0xa01,          /* block ID (uint32_t) */
        0,              /* outgoing block ID (uint32_t) */
        EEP_STORAGE_ID  /* outgoing storage index (uint8_t) */
    },
    {
        0xa02,
        1,
        EEP_STORAGE_ID
    },
    {
        0xb01,
        2,
        EEP_STORAGE_ID
    },
    {
        0xb02,
        0,
        FEE_STORAGE_ID
    },
};

static constexpr ::storage::EepBlockConfig EEP_BLOCK_CONFIG[] = {
    {
        10,   /* EEPROM address (uint32_t) */
        8,    /* size in bytes (uint16_t; without the 4-byte header) */
        true  /* error detection enabled? (bool) */
    },
    {
        22,
        1,
        false
    },
    {
        23,
        5,
        false
    },
};
// END config
// clang-format on

class StorageSystem : public ::lifecycle::AsyncLifecycleComponent
{
public:
    explicit StorageSystem(
        ::async::ContextType driverContext,
        ::async::ContextType userContext,
        ::eeprom::IEepromDriver& eepDriver);
    StorageSystem(StorageSystem const&)            = delete;
    StorageSystem& operator=(StorageSystem const&) = delete;

    void init() final;
    void run() final;
    void shutdown() final;

    ::storage::IStorage& getStorage() { return _mappingStorage; }

private:
    // BEGIN declaration
    ::eeprom::IEepromDriver& _eepDriver;

    static constexpr size_t EEP_CONFIG_SIZE
        = sizeof(EEP_BLOCK_CONFIG) / sizeof(::storage::EepBlockConfig);

    static constexpr size_t MAX_DATA_SIZE = 8; // largest size defined in EEP_BLOCK_CONFIG

    ::storage::declare::EepStorage<EEP_CONFIG_SIZE, MAX_DATA_SIZE> _eepStorage;
    ::storage::FeeStorage _feeStorage;

    ::storage::QueuingStorage _eepQueuingStorage;
    ::storage::QueuingStorage _feeQueuingStorage;

    static constexpr size_t MAPPING_CONFIG_SIZE
        = sizeof(MAPPING_CONFIG) / sizeof(::storage::MappingConfig);

    ::storage::declare::MappingStorage<
        MAPPING_CONFIG_SIZE,
        2 /* number of delegate storages */,
        2 /* max simultaneous jobs */>
        _mappingStorage;

    ::storage::declare::StorageTester<MAX_DATA_SIZE> _storageTester;
    ::console::AsyncCommandWrapper _asyncStorageTester;
    // END declaration
};

} // namespace systems
