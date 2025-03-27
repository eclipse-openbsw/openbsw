// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#pragma once

#include "async/Types.h"

namespace async
{
/**
 * Execute the given runnable immediately, non-blocking call
 * \param context Context of execution
 * \param runnable Runnable to execute
 */
void execute(ContextType context, RunnableType& runnable);

/**
 * Execute runnable after specified delay, non-blocking call
 * \param context Context of execution
 * \param runnable Runnable to execute
 * \param timeout Timeout object
 * \param delay Delay in time units
 * \param unit Time unit, a scaling factor for delay
 */
void schedule(
    ContextType context,
    RunnableType& runnable,
    TimeoutType& timeout,
    uint32_t delay,
    TimeUnitType unit);

/**
 * Set runnable to periodic execution, non-blocking call
 * \param context Context of execution
 * \param runnable Runnable to execute
 * \param timeout Timeout object
 * \param period Period in time units
 * \param unit Time unit, a scaling factor for period
 */
void scheduleAtFixedRate(
    ContextType context,
    RunnableType& runnable,
    TimeoutType& timeout,
    uint32_t period,
    TimeUnitType unit);

} // namespace async
