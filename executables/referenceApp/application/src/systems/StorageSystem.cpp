// Copyright 2025 Accenture.

#include "systems/StorageSystem.h"

namespace systems
{

// BEGIN initialization
StorageSystem::StorageSystem(::async::ContextType context, ::eeprom::IEepromDriver& eepDriver)
: _eepDriver(eepDriver)
, _eepStorage(EEP_BLOCK_CONFIG, _eepDriver)
, _feeStorage()
, _eepQueuingStorage(_eepStorage, context)
, _feeQueuingStorage(_feeStorage, context)
, _mappingStorage(MAPPING_CONFIG, context, _eepQueuingStorage, _feeQueuingStorage)
{
    setTransitionContext(context);
}

void StorageSystem::init() { transitionDone(); }

void StorageSystem::run() { transitionDone(); }

// END initialization

void StorageSystem::shutdown() { transitionDone(); }

} // namespace systems
