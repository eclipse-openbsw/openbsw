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

#include "someip/SdConstants.h"

#include <cstdint>

namespace someip
{
struct SdConfig
{
    uint32_t _initialDelay         = SD_DEFAULT_INITIAL_DELAY;
    uint32_t _repetitionsBaseDelay = SD_DEFAULT_REPETITIONS_BASE_DELAY;
    uint32_t _repetitionsMax       = SD_DEFAULT_REPETITIONS_MAX;

    SdConfig() = default;

    SdConfig(
        uint32_t const initialDelay,
        uint32_t const repetitionsBaseDelay,
        uint32_t const repetitionsMax)
    : _initialDelay(initialDelay)
    , _repetitionsBaseDelay(repetitionsBaseDelay)
    , _repetitionsMax(repetitionsMax)
    {}
};

struct SdOfferConfig : SdConfig
{
    uint32_t _cyclicOfferDelay = SD_DEFAULT_CYCLIC_OFFER_DELAY;

    SdOfferConfig() = default;

    SdOfferConfig(
        uint32_t const initialDelay,
        uint32_t const repetitionsBaseDelay,
        uint32_t const repetitionsMax,
        uint32_t const cyclicOfferDelay)
    : SdConfig(initialDelay, repetitionsBaseDelay, repetitionsMax)
    , _cyclicOfferDelay(cyclicOfferDelay)
    {}
};
} // namespace someip
