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

#include <etl/span.h>

namespace someip
{
class SdOptions
{
public:
    SdOptions() = default;

    bool init(::etl::span<uint8_t const> const& payload, size_t entriesLength);

    void readIndexValues(::etl::span<uint8_t const> const& entry);

    // for now makes these available
    ::etl::span<uint8_t const> getOptionsData() const;

    uint8_t getOptions1Index() const;
    uint8_t getOptions2Index() const;
    uint8_t getOptions1Num() const;
    uint8_t getOptions2Num() const;

private:
    ::etl::span<uint8_t const> _optionsData;

    uint8_t _options1Index = 0xFFU;
    uint8_t _options2Index = 0xFFU;
    uint8_t _options1Num   = 0xFFU;
    uint8_t _options2Num   = 0xFFU;
};

inline ::etl::span<uint8_t const> SdOptions::getOptionsData() const { return _optionsData; }

inline uint8_t SdOptions::getOptions1Index() const { return _options1Index; }

inline uint8_t SdOptions::getOptions2Index() const { return _options2Index; }

inline uint8_t SdOptions::getOptions1Num() const { return _options1Num; }

inline uint8_t SdOptions::getOptions2Num() const { return _options2Num; }

} // namespace someip
