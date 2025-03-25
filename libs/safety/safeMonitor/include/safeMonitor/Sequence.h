// Copyright 2025 Accenture.

#pragma once

#include "common.h"

#include <platform/config.h>
#include <platform/estdint.h>

namespace safeMonitor
{
/**
 * Sequence monitor
 * \tparam Handler A class which provides a handler method
 * \tparam Event The type of event the handler method accepts
 * \tparam Checkpoint The enum type which represents the checkpoints
 * \tparam ScopedMutex A class for making calls to the interface mutually exclusive
 * \tparam Context Type of a context which can be stored in the monitor
 */
template<
    typename Handler,
    typename Event,
    typename Checkpoint,
    typename ScopedMutex = DefaultMutex,
    typename Context     = DefaultContext>
class Sequence
{
public:
    /**
     * \brief Constructor
     * \note The caller must ensure that the reference to the handler remains valid throughout the
     *      lifetime of this objects instance.
     * \param[in] handler Object that provides the handler method
     * \param[in] event Event to pass to the handler method in case of a timeout
     * \param[in] firstCheckpoint First checkpoint in sequence
     * \param[in] lastCheckpoint Last checkpoint in sequence
     */
    constexpr Sequence(
        Handler& handler,
        Event const& event,
        Checkpoint const& firstCheckpoint,
        Checkpoint const& lastCheckpoint)
    : _handler(handler)
    , _event(event)
    , _firstCheckpoint(firstCheckpoint)
    , _lastCheckpoint(lastCheckpoint)
    , _expectedCheckpoint(firstCheckpoint)
    , _context(DEFAULT_CONTEXT)
    {}

    /**
     * \brief Hits a checkpoint
     * \details Call this method in order to hit a specific checkpoint. The method checks if the
     *      given checkpoint is next in the sequence. If it's not, the handler method gets called.
     *      After the last checkpoint was hit, the sequence wraps back to the first checkpoint.
     * \note The operation is guarded by an instance of the ScopedMutex type which was passed as a
     *      template argument. However, the handler function is called outside this ScopedMutex.
     * \note The handler method is called from the same context as this method.
     * \param[in] checkpoint The checkpoint to be hit
     * \param[in] context The context to be stored within the monitor. It is copied to an internal
     *      member variable. Therefore the caller doesn't have to worry about the references
     *      lifetime.
     */
    void hit(Checkpoint const& checkpoint, Context const& context = DEFAULT_CONTEXT)
    {
        ESR_UNUSED const ScopedMutex m;

        _context = context;

        if (checkpoint != _expectedCheckpoint)
        {
            _handler.handle(_event);
        }
        else if (checkpoint == _lastCheckpoint)
        {
            _expectedCheckpoint = _firstCheckpoint;
        }
        else
        {
            _expectedCheckpoint
                = static_cast<Checkpoint>(static_cast<size_t>(_expectedCheckpoint) + 1U);
        }
    }

    /**
     * \brief Gives access to the monitors internal context object
     */
    Context const& getContext() const { return _context; }

private:
    Handler& _handler;
    Event const _event;
    Checkpoint const _firstCheckpoint;
    Checkpoint const _lastCheckpoint;
    Checkpoint _expectedCheckpoint;
    Context _context;
    static Context const DEFAULT_CONTEXT;
};

template<
    typename Handler,
    typename Event,
    typename Checkpoint,
    typename ScopedMutex,
    typename Context>
Context const Sequence<Handler, Event, Checkpoint, ScopedMutex, Context>::DEFAULT_CONTEXT{};

} // namespace safeMonitor
