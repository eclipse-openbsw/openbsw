// Copyright 2025 Accenture.

#pragma once

#include <platform/estdint.h>

namespace safety
{
class SafeSystem
{
    // [METHODS_START]
public:
    /**
     * Initializes the SafeSystem instance, setting up necessary variables and states.
     */
    SafeSystem();

    /**
     * This method prepares the system for voltage monitoring. It should be called once
     * during system initialization, typically from the safetyManager's init function.
     */
    void init();

    /**
     * This method checks the ADC reference voltage and internal supply voltages
     * to ensure they are within safe operating ranges. It should be called cyclically, typically
     * from the safetyManager's cyclic function.
     */
    void cyclic();

private:
    /**
     * Compares the raw ADC value with predefined thresholds to ensure the ADC reference voltage
     * remains within acceptable limits. If is outside the range, it will trigger
     * adcReferenceMonitor to take appropriate action via safetySupervisor.
     */
    void checkAdcReference(uint32_t bandgapRawValue);

    /**
     * Ensures that critical internal supply voltages such as 3.3V Flash, 3.3V Oscillator,
     * and 1.2V Core are within safe operating ranges. If any voltage is outside the range, it will
     * trigger internalSuppliesMonitor to take appropriate action via safetySupervisor.
     */
    void checkInternalSupplies(uint32_t internalSupplyRawValue);

    /**
     * Returns the raw ADC value representing the ADC reference voltage.
     */
    uint32_t getBandgapRawValue();

    /**
     * Returns the raw ADC value representing the internal supply voltage.
     */
    uint32_t getInternalSupplyRawValue();
    // [METHODS_END]

    uint32_t _counter;
    uint32_t _adcRef;
};

} // namespace safety
