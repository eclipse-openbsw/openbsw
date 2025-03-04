// Copyright 2025 Accenture.

#ifndef GUARD_1D7A3C52_59EE_4905_A3F1_8019F098083E
#define GUARD_1D7A3C52_59EE_4905_A3F1_8019F098083E

#include "common.h"

#include <platform/config.h>
#include <platform/estdint.h>

namespace safeMonitor
{
/**
 * Watchdog monitor
 * \tparam Handler A class which provides a handler method
 * \tparam Event The type of event the handler method accepts
 * \tparam ScopedMutex A class for making calls to the interface mutually exclusive
 * \tparam Context Type of a context which can be stored in the monitor
 */
template<
    typename Handler,
    typename Event,
    typename ScopedMutex = DefaultMutex,
    typename Context     = DefaultContext>
class Watchdog
{
public:
    /**
     * \brief Type of Timeout
     * \details This type is used for counting.
     */
    using Timeout = uint32_t;

    /**
     * \brief Constructor
     * \note The caller must ensure that the reference to the handler remains valid throughout the
     *      lifetime of this objects instance.
     * \param[in] handler Object that provides the handler method
     * \param[in] event Event to pass to the handler method in case of a timeout
     * \param[in] timeout Number of calls of the service method until the handler is triggered
     */
    constexpr Watchdog(Handler& handler, Event const& event, Timeout const timeout)
    : _handler(handler)
    , _event(event)
    , _timeout(timeout)
    , _counter(timeout)
    , _context(DEFAULT_CONTEXT)
    {}

    /**
     * \brief Kicks the watchdog.
     * \details Use this function to reset the watchdog. It does so by setting the internal counter
     *      to the timeout value.
     * \note The operation is guarded by an instance of the ScopedMutex type which was passed as a
     *      template argument.
     * \param[in] context The context to be stored within the monitor. It is copied to an internal
     *      member variable. Therefore the caller doesn't have to worry about the references
     *      lifetime.
     */
    void kick(Context const& context = DEFAULT_CONTEXT)
    {
        ESR_UNUSED const ScopedMutex m;
        _context = context;
        _counter = _timeout;
    }

    /**
     * \brief Services the watchdog
     * \details This method is meant to be called in a cyclic fashion. Cycle time (T) and the value
     *      given to the constructor as timeout determine the time window (t) of the watchdog:
     *      _t = T * timeout_. Each time the service method is called, it decrements an internal
     *      counter. Once that counter reaches a value of 0, the handler gets called in order to
     *      signal a timeout.
     * \note The operation is guarded by an instance of the ScopedMutex type which was passed as a
     *      template argument. However, the handler function is called outside this ScopedMutex.
     * \note The handler method is called from the same context as this method.
     * \warning Make sure that the call of this function is not blocked by the algorithm to be
     *      monitored or similar. Otherwise a timeout will be detected too late or possibly not at
     *      all. A possible solution would be to call the service method from a task with higher
     *      priority than the algorithm runs in.
     */
    void service()
    {
        ESR_UNUSED const ScopedMutex m;

        if (_counter > 0U)
        {
            _counter--;
        }

        if (_counter == 0U)
        {
            _handler.handle(_event);
        }
    }

    /**
     * \brief Gives access to the monitors internal context object
     */
    Context const& getContext() const { return _context; }

private:
    Handler& _handler;
    Event const _event;
    Timeout const _timeout;
    Timeout _counter;
    Context _context;
    static Context const DEFAULT_CONTEXT;
};

template<typename Handler, typename Event, typename ScopedMutex, typename Context>
Context const Watchdog<Handler, Event, ScopedMutex, Context>::DEFAULT_CONTEXT{};

} // namespace safeMonitor

#endif // GUARD_1D7A3C52_59EE_4905_A3F1_8019F098083E
