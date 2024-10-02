// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#ifndef GUARD_44C9F9DB_11A5_4094_AB12_8B02A7C1F937
#define GUARD_44C9F9DB_11A5_4094_AB12_8B02A7C1F937

#include "async/Types.h"

namespace async
{
template<typename EventDispatcher, size_t Event>
class EventPolicy
{
public:
    using EventDispatcherType = EventDispatcher;
    using HandlerFunctionType = typename EventDispatcher::HandlerFunctionType;

    explicit EventPolicy(EventDispatcher& eventDispatcher);

    void setEventHandler(HandlerFunctionType handlerFunction);
    void removeEventHandler();

    void setEvent();

private:
    static EventMaskType const _eventMask = static_cast<EventMaskType>(1U << Event);

    EventDispatcher& _eventDispatcher;
};

/**
 * Inline implementations.
 */
template<typename EventDispatcher, size_t Event>
inline EventPolicy<EventDispatcher, Event>::EventPolicy(EventDispatcher& eventDispatcher)
: _eventDispatcher(eventDispatcher)
{}

template<typename EventDispatcher, size_t Event>
inline void
EventPolicy<EventDispatcher, Event>::setEventHandler(HandlerFunctionType const handlerFunction)
{
    _eventDispatcher.setEventHandler(Event, handlerFunction);
}

template<typename EventDispatcher, size_t Event>
inline void EventPolicy<EventDispatcher, Event>::removeEventHandler()
{
    _eventDispatcher.removeEventHandler(Event);
}

template<typename EventDispatcher, size_t Event>
inline void EventPolicy<EventDispatcher, Event>::setEvent()
{
    _eventDispatcher.setEvents(_eventMask);
}

} // namespace async

#endif // GUARD_44C9F9DB_11A5_4094_AB12_8B02A7C1F937
