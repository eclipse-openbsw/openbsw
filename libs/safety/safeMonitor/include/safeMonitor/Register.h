// Copyright 2025 Accenture.

#pragma once

#include "common.h"

#include <platform/config.h>
#include <platform/estdint.h>

namespace safeMonitor
{
/**
 * Register monitor
 * \tparam Handler A class which provides a handler method
 * \tparam Event The type of event the handler method accepts
 * \tparam RegisterType Type of a register to be checked
 * \tparam ScopedMutex A class for making calls to the interface mutually exclusive
 * \tparam Context Type of a context which can be stored in the monitor
 */
template<
    typename Handler,
    typename Event,
    typename RegisterType,
    typename ScopedMutex = DefaultMutex,
    typename Context     = DefaultContext>
class Register
{
public:
    /**
     * A configuration entry representing one register to check
     */
    struct Entry
    {
        uintptr_t address;
        RegisterType valueMask;
        RegisterType expectedValue;

        bool isExpectedValue() const { return expectedValue == (readValue() & valueMask); }

        RegisterType readValue() const
        {
            return *reinterpret_cast<RegisterType const volatile*>(address);
        }
    };

    /**
     * \brief Constructor
     * \note The caller must ensure that the reference to the handler remains valid throughout the
     *      lifetime of this objects instance.
     * \note The entries array is _NOT_ copied but referenced. Therefore reference must also remain
     *      valid throughout the objects lifetime.
     * \param[in] handler Object that provides the handler method
     * \param[in] event Event to pass to the handler method in case of a timeout
     * \param[in] entries Reference to an array of Entries to scan
     * \tparam numberOfEntries Number of entries in the referenced array. This normally gets
     *      deducted automatically (see usage example).
     */
    template<size_t numberOfEntries>
    constexpr Register(
        Handler& handler, Event const& event, Entry const (&entries)[numberOfEntries])
    : _handler(handler)
    , _event(event)
    , _entries(&entries[0])
    , _numberOfEntries(numberOfEntries)
    , _context(DEFAULT_CONTEXT)
    , _lastCheckedEntry(nullptr)
    {}

    /**
     * \brief Checks all entries
     * \details Goes through the list of register entries and checks if the value matches with the
     *      expected one. If a mismatch is detected, the handler gets called and the remaining
     *      entries are _NOT_ checked.
     * \note The operation is guarded by an instance of the ScopedMutex type which was passed as a
     *      template argument. However, the handler function is called outside this ScopedMutex.
     * \note The handler method is called from the same context as this method.
     * \param[in] context The context to be stored within the monitor. It is copied to an internal
     *      member variable. Therefore the caller doesn't have to worry about the references
     *      lifetime.
     */
    void check(Context const& context = DEFAULT_CONTEXT)
    {
        ESR_UNUSED const ScopedMutex m;

        _context    = context;
        bool doFire = false;

        for (size_t i = 0U; (i < _numberOfEntries) && (!doFire); ++i)
        {
            doFire            = !(_entries[i].isExpectedValue());
            _lastCheckedEntry = &_entries[i];
        }

        if (doFire)
        {
            _handler.handle(_event);
        }
    }

    /**
     * \brief Gives access to the monitors internal context object
     */
    Context const& getContext() const { return _context; }

    /**
     * \brief Returns the entry that was last checked. Use this to access the faulty entry in case
     * of an error.
     */
    Entry const* getLastCheckedEntry() const { return _lastCheckedEntry; }

private:
    Handler& _handler;
    Event const _event;
    Entry const* const _entries;
    size_t const _numberOfEntries;
    Context _context;
    Entry const* _lastCheckedEntry;
    static Context const DEFAULT_CONTEXT;
};

template<
    typename Handler,
    typename Event,
    typename RegisterType,
    typename ScopedMutex,
    typename Context>
Context const Register<Handler, Event, RegisterType, ScopedMutex, Context>::DEFAULT_CONTEXT{};

} // namespace safeMonitor
