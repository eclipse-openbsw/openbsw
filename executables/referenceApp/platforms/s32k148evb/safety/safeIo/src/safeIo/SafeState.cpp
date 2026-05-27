/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "safeIo/SafeState.h"

#include <io/Io.h>

namespace safety
{

namespace
{
constexpr auto SAFETY_PIN_ID          = ::bios::Io::EVAL_LED_RED;
constexpr uint32_t SAFETY_PIN_DISABLE = static_cast<uint32_t>(::bios::Io::PINDISABLE);
constexpr uint32_t SAFETY_PIN_ENABLE
    = static_cast<uint32_t>(::bios::Io::GPIO) | static_cast<uint32_t>(::bios::Io::STRENGTH_ON);
} // namespace

void SafeState::enterSafeState()
{
    ::bios::Io::PinConfiguration pinConfig;
    ::bios::Io::getConfiguration(SAFETY_PIN_ID, pinConfig);
    pinConfig.pinCfg = SAFETY_PIN_DISABLE;
    ::bios::Io::setConfiguration(SAFETY_PIN_ID, pinConfig);
}

void SafeState::leaveSafeState()
{
    ::bios::Io::PinConfiguration pinConfig;
    ::bios::Io::getConfiguration(SAFETY_PIN_ID, pinConfig);
    pinConfig.pinCfg = SAFETY_PIN_ENABLE;
    ::bios::Io::setConfiguration(SAFETY_PIN_ID, pinConfig);
}

} // namespace safety
