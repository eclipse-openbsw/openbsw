// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#ifndef GUARD_2A2B08B1_09D6_4398_8B59_9E02EDE40F4D
#define GUARD_2A2B08B1_09D6_4398_8B59_9E02EDE40F4D

#include "async/Types.h"

namespace async
{
void execute(ContextType context, RunnableType& runnable);
void schedule(
    ContextType context,
    RunnableType& runnable,
    TimeoutType& timeout,
    uint32_t delay,
    TimeUnitType unit);
void scheduleAtFixedRate(
    ContextType context,
    RunnableType& runnable,
    TimeoutType& timeout,
    uint32_t period,
    TimeUnitType unit);

} // namespace async

#endif // GUARD_2A2B08B1_09D6_4398_8B59_9E02EDE40F4D
