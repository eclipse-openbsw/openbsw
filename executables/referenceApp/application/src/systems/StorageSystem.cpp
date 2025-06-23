// Copyright 2025 Accenture.

#include "systems/StorageSystem.h"

namespace systems
{

// BEGIN initialization
StorageSystem::StorageSystem(
    ::async::ContextType const driverContext,
    ::async::ContextType const userContext,
    ::eeprom::IEepromDriver& eepDriver)
: _eepDriver(eepDriver)
, _eepStorage(EEP_BLOCK_CONFIG, _eepDriver)
, _feeStorage()
, _eepQueuingStorage(_eepStorage, driverContext)
, _feeQueuingStorage(_feeStorage, driverContext)
, _mappingStorage(MAPPING_CONFIG, driverContext, _eepQueuingStorage, _feeQueuingStorage)
, _storageTester(_mappingStorage, driverContext)
, _asyncStorageTester(_storageTester, userContext)
{
    setTransitionContext(driverContext);
}

void StorageSystem::init() { transitionDone(); }

void StorageSystem::run() { transitionDone(); }

// END initialization

void StorageSystem::shutdown() { transitionDone(); }

} // namespace systems
