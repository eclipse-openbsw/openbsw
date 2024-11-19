// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#ifndef GUARD_DDD1DD79_9BAA_457F_BFAA_C237BA5358CC
#define GUARD_DDD1DD79_9BAA_457F_BFAA_C237BA5358CC

#include "async/Types.h"

#include <estd/array.h>
#include <estd/functional.h>
#include <platform/config.h>

namespace async
{
/**
 * A template class that allows a handler function to be registered or deregistered, and then
 * executed based on the provided eventMask. Each event corresponds to a bit in the eventMask. The
 * handler function is executed when the specified bit in the eventMask is set.
 *
 * \tparam EventCount Number of events to be handled.
 * \tparam Lock RAII lock to support concurrent execution.
 */
template<size_t EventCount, typename Lock>
class EventDispatcher
{
public:
    static size_t const EVENT_COUNT = EventCount;
    using LockType                  = Lock;
    using HandlerFunctionType       = ::estd::function<void()>;

    void setEventHandler(size_t event, HandlerFunctionType handlerFunction);
    void removeEventHandler(size_t event);

    void handleEvents(EventMaskType eventMask) const;

private:
    using HandlerFunctionsArrayType = ::estd::array<HandlerFunctionType, EventCount>;

    template<size_t Event, bool IsValid = (Event < EventCount)>
    struct EventDispatcherImpl
    {
        static EventMaskType const EVENT_MASK = static_cast<EventMaskType>(1U << Event);

        static void
        handleEvents(HandlerFunctionsArrayType const& handlerFunctions, EventMaskType eventMask);
    };

    template<size_t Event>
    struct EventDispatcherImpl<Event, false>
    {
        static void
        handleEvents(HandlerFunctionsArrayType const& handlerFunctions, EventMaskType eventMask);
    };

    HandlerFunctionsArrayType _handlerFunctions;
};

/**
 * Inline implementations.
 */
template<size_t EventCount, typename Lock>
inline void EventDispatcher<EventCount, Lock>::setEventHandler(
    size_t const event, HandlerFunctionType const handlerFunction)
{
    ESR_UNUSED const Lock lock;
    _handlerFunctions[event] = handlerFunction;
}

template<size_t EventCount, typename Lock>
inline void EventDispatcher<EventCount, Lock>::removeEventHandler(size_t const event)
{
    ESR_UNUSED const Lock lock;
    _handlerFunctions[event] = HandlerFunctionType();
}

template<size_t EventCount, typename Lock>
inline void EventDispatcher<EventCount, Lock>::handleEvents(EventMaskType const eventMask) const
{
    EventDispatcherImpl<0U>::handleEvents(_handlerFunctions, eventMask);
}

template<size_t EventCount, typename Lock>
template<size_t Event, bool IsValid>
inline void EventDispatcher<EventCount, Lock>::EventDispatcherImpl<Event, IsValid>::handleEvents(
    HandlerFunctionsArrayType const& handlerFunctions, EventMaskType const eventMask)
{
    if ((eventMask & EVENT_MASK) != 0U)
    {
        handlerFunctions[Event]();
    }
    EventDispatcherImpl<Event + 1U>::handleEvents(handlerFunctions, eventMask);
}

template<size_t EventCount, typename Lock>
template<size_t Event>
inline void EventDispatcher<EventCount, Lock>::EventDispatcherImpl<Event, false>::handleEvents(
    HandlerFunctionsArrayType const& /*handlerFunctions*/, EventMaskType const /*eventMask*/)
{}

} // namespace async

#endif // GUARD_DDD1DD79_9BAA_457F_BFAA_C237BA5358CC
