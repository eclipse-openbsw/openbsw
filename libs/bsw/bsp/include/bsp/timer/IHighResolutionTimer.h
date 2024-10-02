// Copyright 2024 Accenture.

#ifndef GUARD_403616B7_511B_410C_9E29_0260013EC0AF
#define GUARD_403616B7_511B_410C_9E29_0260013EC0AF

#include <estd/functional.h>
#include <platform/estdint.h>

namespace bios
{
class IHighResolutionTimer
{
public:
    /**
     * Start timer ( if timer is running then it will be stopped and started again ).
     * \param d  function to be called if timer expired
     * \param timeInMkSec Time in mkSec. MAX value is dependent on implementation.
     */
    virtual void set(::estd::function<void()>& d, uint32_t timeInMkSec) = 0;
    /**
     * Cancel timer.
     */
    virtual void cancel()                                               = 0;

protected:
    IHighResolutionTimer& operator=(IHighResolutionTimer const&) = default;
};

} // namespace bios

#endif
