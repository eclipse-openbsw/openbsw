// Copyright 2025 Accenture.

#include "safeSystem/SafeSystem.h"

#include <bsp/adc/AnalogInput.h>
#include <interrupts/SuspendResumeAllInterruptsScopedLock.h>
#include <mcu/mcu.h>
#include <safeMemory/MemoryProtection.h>
#include <safeSupervisor/SafeSupervisor.h>

namespace safety
{
SafeSystem::SafeSystem() : _counter(0U), _adcRef(5000U) {}

void SafeSystem::init()
{
    SIM->CHIPCTL = (SIM->CHIPCTL & (~SIM_CHIPCTL_ADC_SUPPLYEN_MASK))
                   | (1 << SIM_CHIPCTL_ADC_SUPPLYEN_SHIFT); // Enable ADC supply monitor
}

void SafeSystem::cyclic()
{
    ::interrupts::SuspendResumeAllInterruptsScopedLock const lock;
    MemoryProtection::fusaGateClose();
    auto const bandgapRawValue        = getBandgapRawValue();
    auto const internalSupplyRawValue = getInternalSupplyRawValue();
    MemoryProtection::fusaGateOpen();
    checkAdcReference(bandgapRawValue);
    checkInternalSupplies(internalSupplyRawValue);
}

void SafeSystem::checkAdcReference(uint32_t const bandgapRawValue)
{
    constexpr uint32_t adcMaxRaw      = 0xFFFU; // 12-bit ADC
    constexpr uint32_t bandgapVoltage = 1000U;
    _adcRef                           = bandgapVoltage * adcMaxRaw / bandgapRawValue;
    if ((_adcRef < 4800U) || (_adcRef > 5200U))
    {
        auto& supervisor = safety::SafeSupervisor::getInstance();
        supervisor.adcReferenceMonitor.trigger();
    }
}

void SafeSystem::checkInternalSupplies(uint32_t const internalSupplyRawValue)
{
    constexpr uint32_t VDD_3V3_OSC   = 3U;
    constexpr uint32_t VDD_3V3_FLASH = 4U;
    constexpr uint32_t VDD_1V2_CORE  = 5U;
    uint32_t const mask              = SIM_CHIPCTL_ADC_SUPPLY_MASK;
    uint32_t const shift             = SIM_CHIPCTL_ADC_SUPPLY_SHIFT;
    switch (_counter)
    {
        case 0:
        {
            SIM->CHIPCTL = (SIM->CHIPCTL & (~mask))
                           | (VDD_3V3_FLASH << shift); // Set ADC supply monitor to 3.3V Flash
            break;
        }
        case 2:
        {
            SIM->CHIPCTL = (SIM->CHIPCTL & (~mask))
                           | (VDD_3V3_OSC << shift); // Set ADC supply monitor to 3.3V Oscillator
            break;
        }
        case 4:
        {
            SIM->CHIPCTL = (SIM->CHIPCTL & (~mask))
                           | (VDD_1V2_CORE << shift); // Set ADC supply monitor to 1.2V Core
            break;
        }
        default:
        {
            constexpr uint32_t adcMaxRaw = 0xFFFU; // 12-bit ADC
            uint32_t const supply        = (SIM->CHIPCTL & mask) >> shift;
            uint32_t const ref           = (supply == VDD_1V2_CORE) ? 1200U : 3300U;
            uint32_t const supplyVoltage = _adcRef * internalSupplyRawValue / adcMaxRaw;
            if ((supplyVoltage < (ref - 200U)) || (supplyVoltage > (ref + 200U)))
            {
                auto& supervisor = safety::SafeSupervisor::getInstance();
                supervisor.internalSuppliesMonitor.trigger();
            }
            break;
        }
    }
    _counter = (_counter < 5U) ? (_counter + 1U) : 0U;
}

uint32_t SafeSystem::getBandgapRawValue()
{
    uint32_t value = 0;
    (void)::bios::AnalogInput::get(::bios::AnalogInput::Ai_BANDGAP, value);
    return value;
}

uint32_t SafeSystem::getInternalSupplyRawValue()
{
    uint32_t value = 0;
    (void)::bios::AnalogInput::get(::bios::AnalogInput::Ai_ADCINTERNAL0, value);
    return value;
}

} // namespace safety
