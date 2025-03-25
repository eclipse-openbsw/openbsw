// Copyright 2025 Accenture.

#pragma once

#include "common.h"

#include <platform/config.h>

namespace safeMonitor
{
/**
 * Value monitor
 * \tparam Handler A class which provides a handler method
 * \tparam Event The type of event the handler method accepts
 * \tparam ValueType Type of the value to be checked
 * \tparam ScopedMutex A class for making calls to the interface mutually exclusive
 * \tparam Context Type of a context which can be stored in the monitor
 */
template<
    typename Handler,
    typename Event,
    typename ValueType,
    typename ScopedMutex = DefaultMutex,
    typename Context     = DefaultContext>
class Value
{
public:
    /**
     * \brief Type of the value to be checked
     */
    using Type = ValueType;

    /**
     * \brief Constructor
     * \note The caller must ensure that the reference to the handler remains valid throughout the
     *      lifetime of this objects instance.
     * \param[in] handler Object that provides the handler method
     * \param[in] event Event to pass to the handler method
     * \param[in] expectedValue Expected value
     */
    constexpr Value(Handler& handler, Event const& event, Type const& expectedValue)
    : _handler(handler), _event(event), _expectedValue(expectedValue), _context(DEFAULT_CONTEXT)
    {}

    /**
     * \brief Checks the given value
     * \details Checks if the given value matches the one which was passed to the constructor
     *      (expectedValue). If it doesn't, the handler gets called.
     * \note The handler method is called from the same context as this method.
     * \param[in] value The value to check
     * \param[in] context The context to be stored within the monitor. It is copied to an internal
     *      member variable. Therefore the caller doesn't have to worry about the references
     *      lifetime.
     */
    void check(Type const& value, Context const& context = DEFAULT_CONTEXT)
    {
        ESR_UNUSED const ScopedMutex m;

        _context = context;

        if (value != _expectedValue)
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
    Type const _expectedValue;
    Context _context;
    static Context const DEFAULT_CONTEXT;
};

template<
    typename Handler,
    typename Event,
    typename ValueType,
    typename ScopedMutex,
    typename Context>
Context const Value<Handler, Event, ValueType, ScopedMutex, Context>::DEFAULT_CONTEXT{};

} // namespace safeMonitor

