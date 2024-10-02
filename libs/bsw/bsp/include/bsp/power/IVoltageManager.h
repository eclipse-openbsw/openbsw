// Copyright 2024 Accenture.

#ifndef GUARD_C2531EBA_CDC3_4EAF_87D8_6860F0ECF78B
#define GUARD_C2531EBA_CDC3_4EAF_87D8_6860F0ECF78B

#include <platform/estdint.h>

namespace bsp
{
class AbstractPowerStateListener;
class IUnderVoltageListener;

/**
 * Interface of a class which provides Information about the voltage on the board.
 */
class IVoltageManager
{
public:
    /**
     * Registers a PowerStateListener at the VoltageManager.
     * \param listener The listener to register
     * \param triggerVoltage The trigger voltage in 1/100 Volts (i.e. 7.07V == 707)
     **/
    virtual void
    addPowerStateListener(AbstractPowerStateListener& listener, uint16_t triggerVoltage)
        = 0;

    /**
     * Removes a PowerStateListener from the VoltageManager.
     * \param listener The listener to remove
     **/
    virtual void removePowerStateListener(AbstractPowerStateListener& listener) = 0;

    /**
     * Registers a UnderVoltageListener at the VoltageManager.
     * \param listener The listener to register
     **/
    virtual void addUnderVoltageListener(IUnderVoltageListener& listener) = 0;

    /**
     * Removes a UnderVoltageListener from the VoltageManager.
     * \param listener The listener to remove
     **/
    virtual void removeUnderVoltageListener(IUnderVoltageListener& listener) = 0;

    /**
     * Returns the current voltage.
     * \return The current voltage in 1/100 Volts (i.e. 7.07V == 707)
     **/
    virtual uint16_t getCurrentVoltage() = 0;

    /**
     * Returns information whether undervoltage occurred.
     * \return If undervoltage occurred since last notification over function
     * undervoltageOccurred()
     */
    virtual bool getUndervoltageOccurred() = 0;

protected:
    IVoltageManager& operator=(IVoltageManager const&) = default;
};

} /* namespace bsp */

#endif /* GUARD_C2531EBA_CDC3_4EAF_87D8_6860F0ECF78B */
