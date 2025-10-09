// Copyright 2025 Accenture.

#include "safeIo/SafeIo.h"

#include <io/Io.h>
#include <safeSupervisor/SafeSupervisor.h>

namespace safety
{

void SafeIo::init()
{
    _lastCheckState = true;

    // Lock all safety relevant registers.
    // For demonstration purposes, only the config register of the green LED is locked.
    ::bios::Io::PinConfiguration pinConfig;
    ::bios::Io::getConfiguration(::bios::Io::EVAL_LED_GREEN, pinConfig);
    pinConfig.pinCfg |= ::bios::Io::lock;
    ::bios::Io::setConfiguration(::bios::Io::EVAL_LED_GREEN, pinConfig);

    // It is necessary to check the registers AFTER locking to ensure, that the correct values were
    // locked. For demonstration purposes, only the three MUX bits of the green LED are checked.
    ::bios::Io::getConfiguration(::bios::Io::EVAL_LED_GREEN, pinConfig);
    if (::bios::Io::ALT2 != (pinConfig.pinCfg & ::bios::Io::ALT7))
    {
        auto& supervisor = safety::SafeSupervisor::getInstance();
        supervisor.lockRegisterMonitor.trigger(::bios::Io::EVAL_LED_GREEN);
    }
}

void SafeIo::cyclic()
{
    // Check all safety relevant registers.
    // Depending on the FHTI (Fault Handling Time Interval), not all registers need to be checked
    // every cycle, the checks can be distributed over multiple cycles For demonstration purposes,
    // it is only checked if the pin SAFETY_TEST is still configured as input.
    ::bios::Io::PinConfiguration pinConfigSafetyTest;
    ::bios::Io::getConfiguration(::bios::Io::SAFETY_TEST, pinConfigSafetyTest);
    bool const checkState = (pinConfigSafetyTest.dir == ::bios::Io::_IN);
    if (checkState != _lastCheckState)
    {
        auto& supervisor = safety::SafeSupervisor::getInstance();
        supervisor.checkRegisterMonitor.trigger(checkState);
        _lastCheckState = checkState;
    }
}

} // namespace safety
