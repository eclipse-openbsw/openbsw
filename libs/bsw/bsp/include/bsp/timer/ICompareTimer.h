// Copyright 2024 Accenture.

#ifndef GUARD_F5FB0B5F_D227_4742_A67B_EE4142FFD390
#define GUARD_F5FB0B5F_D227_4742_A67B_EE4142FFD390

#include <platform/estdint.h>

namespace bios
{
class ICompareTimer
{
public:
    /*
     * Init without start.
     */
    virtual void init() = 0;

    /**
     * Start the timer.
     * \param tick_mks,int_enable should be ISR active or not
     */
    virtual uint8_t start(uint32_t tick_mks, bool int_enable) = 0;

    virtual void stop() = 0;

    virtual void clearIsrStatus() = 0;

    virtual bool getIsrStatus() = 0;

    virtual void isrEnable(bool autoClean = true) = 0;

    virtual bool getIsrEnableStatus() = 0;

    virtual void isrDisable(bool autoClean = true) = 0;

    /*
     * Get compare value.
     */
    virtual uint32_t getPeriodTime() = 0;

    /*
     * Get current value.
     */
    virtual uint32_t getCurrentTime() = 0;

protected:
    ICompareTimer& operator=(ICompareTimer const&) = default;
};

} /* namespace bios */

#endif /* GUARD_F5FB0B5F_D227_4742_A67B_EE4142FFD390 */
