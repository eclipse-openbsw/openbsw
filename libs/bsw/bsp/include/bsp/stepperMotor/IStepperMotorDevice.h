// Copyright 2024 Accenture.

#ifndef GUARD_F701F1AB_11D9_4337_87BD_8308B4116343
#define GUARD_F701F1AB_11D9_4337_87BD_8308B4116343

#include <platform/estdint.h>

namespace bios
{
class IStepperMotorDevice
{
public:
    enum tStatus
    {
        OK_On = 0,
        OK_Off,
        OpenLoad1,
        OpenLoad2,
        OpenLoad3,
        OpenLoad4,
        OverCurrent1,
        OverCurrent2,
        OverCurrent3,
        OverCurrent4,
        OverTemperature,
        OverVoltage,
        UnderVoltage,
        HWError
    };

    enum tDir
    {
        DIR_CLOCK = 0,
        DIR_NOTCLOCK
    };

    virtual void init()                                    = 0;
    virtual void shutdown()                                = 0;
    virtual void step()                                    = 0;
    /**
     * Set MAX current, current in motor is step dependent.
     * \param current in mA
     * \return true =job Done.
     */
    virtual bool setCurrent(uint32_t current)              = 0;
    /**
     * Set Dir.
     * \param dir dir to be set.
     * \return true =job Done.
     */
    virtual bool setDir(tDir dir)                          = 0;
    /**
     * Get actual status from device.
     * \return tStatus
     */
    virtual tStatus status()                               = 0;
    /**
     * Get HS resolution.
     * \return how match microsteps is inside from HS, 0 - error
     */
    virtual uint8_t getHalfStepResolution()                = 0;
    /**
     * Get phase position from stepper motor.
     * \return data valid
     * \return phasePosition/Current vector position
     */
    virtual bool getPhasePosition(uint16_t& phasePosition) = 0;
    /**
     * Set phase position for stepper motor.
     * \param phasePosition Current vector position
     * \return true - job done, false communication error
     */
    virtual bool setPhasePosition(uint16_t phasePosition)  = 0;

protected:
    IStepperMotorDevice& operator=(IStepperMotorDevice const&) = default;
};

} // namespace bios

#endif /* GUARD_F701F1AB_11D9_4337_87BD_8308B4116343 */
