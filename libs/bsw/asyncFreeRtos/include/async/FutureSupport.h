// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#pragma once

#include <async/Types.h>
#include <util/concurrent/IFutureSupport.h>

#include <FreeRTOS.h>
#include <event_groups.h>

namespace async
{
/**
 * This class implements wait and notify
 * functions to synchronize between threads.
 *
 */
class FutureSupport : public ::os::IFutureSupport
{
public:
    /**
     * Class constructor.
     *
     * \param context execution context.
     */
    explicit FutureSupport(ContextType context);

    /**
     * Wait function sets the thread into "waiting" state.
     *
     * \param context execution context.
     */
    void wait() override;

    /**
     * Notify function wakes from "waiting" state.
     *
     * \param context execution context.
     */
    void notify() override;

    /**
     * The function causes assertion if context assigned
     * to this instance is not equal to actual context of
     * FreeRTOS' task where functions of current instance are executed.
     */
    void assertTaskContext() override;

    /**
     * The function checks if the context assigned to this instance
     * corresponds to actual context of FreeRTOS' task
     * where functions of current instance are executed.
     *
     * \return true if contexts match.
     */
    bool verifyTaskContext() override;

private:
    ContextType _context;
    StaticEventGroup_t _eventGroup;
    EventGroupHandle_t _eventGroupHandle;
};

} // namespace async

