// Copyright 2025 Accenture.

#pragma once

#include "common.h"

#include <platform/config.h>

namespace safeMonitor
{
/**
 * Trigger monitor
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
class Trigger
{
public:
    /**
     * \brief Constructor
     * \note The caller must ensure that the reference to the handler remains valid throughout the
     *      lifetime of this objects instance.
     * \param[in] handler Object that provides the handler method
     * \param[in] event Event to pass to the handler method in case of a timeout
     */
    constexpr Trigger(Handler& handler, Event const& event)
    : _handler(handler), _event(event), _context(DEFAULT_CONTEXT)
    {}

    /**
     * \brief Triggers the handler with the event
     * \note The handler method is called from the same context as this method.
     * \param[in] context The context to be stored within the monitor. It is copied to an internal
     *      member variable. Therefore the caller doesn't have to worry about the references
     *      lifetime.
     */
    void trigger(Context const& context = DEFAULT_CONTEXT)
    {
        ESR_UNUSED const ScopedMutex m;
        _context = context;
        _handler.handle(_event);
    }

    /**
     * \brief Gives access to the monitors internal context object
     */
    Context const& getContext() const { return _context; }

private:
    Handler& _handler;
    Event const _event;
    Context _context;
    static Context const DEFAULT_CONTEXT;
};

template<typename Handler, typename Event, typename ScopedMutex, typename Context>
Context const Trigger<Handler, Event, ScopedMutex, Context>::DEFAULT_CONTEXT{};

} // namespace safeMonitor

