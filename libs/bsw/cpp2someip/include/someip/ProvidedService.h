/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "someip/IProvidedServiceListener.h"
#include "someip/SdConfig.h"
#include "someip/ServiceDescription.h"
#include "someip/init.h"

#include <cstdint>

namespace someip
{
class ServiceHandler;

class ProvidedService
{
public:
    enum class ProvidedServiceState : uint8_t
    {
        IDLE_PHASE,
        INITIAL_WAIT_PHASE,
        REPETITION_PHASE,
        MAIN_PHASE,
        DENOUNCEMENT_PHASE,
        REMOVAL_PHASE
    };

    explicit ProvidedService(IProvidedServiceListener* listener = nullptr);

    explicit ProvidedService(ServiceHandler& handler, IProvidedServiceListener* listener = nullptr);

    void init();

    ServiceHandler* getHandler() const;
    void setHandler(ServiceHandler& implementation);

    uint64_t getTimestamp() const;
    void setTimestamp(uint64_t timestamp);

    uint32_t getRepetitionCount() const;
    void setRepetitionCount(uint32_t count);

    ProvidedServiceState getState() const;
    void setState(ProvidedServiceState state);

    SdOfferConfig const& getSdConfig() const;
    void setSdConfig(SdOfferConfig const& config);

    void unregisterDone() const;

    ServiceDescription description;

private:
    ServiceHandler* _pHandler;
    IProvidedServiceListener* _pListener;
    uint64_t _timestamp;
    uint32_t _repetitionCount;
    ProvidedServiceState _state;
    SdOfferConfig _sdConfig;
};

/*
 * inline implementation
 */
inline ProvidedService::ProvidedService(IProvidedServiceListener* const listener)
: description()
, _pHandler(nullptr)
, _pListener(listener)
, _timestamp(0U)
, _repetitionCount(0U)
, _state(ProvidedServiceState::IDLE_PHASE)
, _sdConfig()
{}

inline ProvidedService::ProvidedService(
    ServiceHandler& handler, IProvidedServiceListener* const listener)
: description(make<ServiceDescription>())
, _pHandler(&handler)
, _pListener(listener)
, _timestamp(0U)
, _repetitionCount(0U)
, _state(ProvidedServiceState::IDLE_PHASE)
, _sdConfig()
{}

inline void ProvidedService::init()
{
    _timestamp       = 0U;
    _repetitionCount = 0U;
    _state           = ProvidedService::ProvidedServiceState::INITIAL_WAIT_PHASE;
}

inline ServiceHandler* ProvidedService::getHandler() const { return _pHandler; }

inline void ProvidedService::setHandler(ServiceHandler& implementation)
{
    _pHandler = &implementation;
}

inline uint64_t ProvidedService::getTimestamp() const { return _timestamp; }

inline void ProvidedService::setTimestamp(uint64_t const timestamp) { _timestamp = timestamp; }

inline uint32_t ProvidedService::getRepetitionCount() const { return _repetitionCount; }

inline void ProvidedService::setRepetitionCount(uint32_t const count) { _repetitionCount = count; }

inline SdOfferConfig const& ProvidedService::getSdConfig() const { return _sdConfig; }

inline void ProvidedService::setSdConfig(SdOfferConfig const& config) { _sdConfig = config; }

inline ProvidedService::ProvidedServiceState ProvidedService::getState() const { return _state; }

inline void ProvidedService::setState(ProvidedServiceState const state) { _state = state; }

inline void ProvidedService::unregisterDone() const
{
    if (_pListener != nullptr)
    {
        _pListener->unregisterDone(*this);
    }
}

} // namespace someip
