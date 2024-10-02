// Copyright 2024 Accenture.

/**
 * Contains interface every PowerStateListener has to implement.
 * \file AbstractPowerStateListener.h
 * \ingroup bsp
 *
 */

#ifndef GUARD_BA4B5F1D_E80A_4B23_AC16_9154AC14803E
#define GUARD_BA4B5F1D_E80A_4B23_AC16_9154AC14803E

#include <estd/forward_list.h>
#include <platform/estdint.h>

namespace bsp
{
/**
 * @class       AbstractPowerStateListener
 *
 * Interface every PowerStateListener has to implement.
 * The PowerStateListener will be notified if a change in the voltage level beyond the
 * trigger level occurs. The trigger level has to be supplied when the listener
 * is added to the PowerManager
 */
class AbstractPowerStateListener : public ::estd::forward_list_node<AbstractPowerStateListener>
{
public:
    AbstractPowerStateListener();

    /**
     * Notifies the listener of a change in the voltage level beyond the
     * trigger level. The trigger level has to be supplied when the listener
     * is added to the PowerManager.
     * \param currentVoltage current voltage in 1/100 Volts (i.e. 7.07V == 707)
     */
    virtual void powerStateChanged(uint16_t currentVoltage) = 0;

private:
    friend class VoltageManager;
    uint16_t fTriggerVoltage;

    uint16_t getTriggerVoltage() const;
};

/*
 * inline implementation
 */
inline AbstractPowerStateListener::AbstractPowerStateListener() : fTriggerVoltage(0U) {}

inline uint16_t AbstractPowerStateListener::getTriggerVoltage() const { return fTriggerVoltage; }

} /* namespace bsp */

#endif /* GUARD_BA4B5F1D_E80A_4B23_AC16_9154AC14803E */
