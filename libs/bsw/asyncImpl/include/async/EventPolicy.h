/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

/**
 * \ingroup async
 */
#pragma once

#include "async/Types.h"

namespace async
{
/**
 * A template class that encapsulates specific event behavior, allowing setEvent to be used without
 * explicitly specifying the eventMask.
 *
 * \tparam EventDispatcher EventDispatcher class, from which the Event is encapsulated.
 * \tparam Event Event number to encapsulate.
 */
template<typename EventDispatcher, size_t Event>
class EventPolicy
{
public:
    using EventDispatcherType = EventDispatcher;
    using HandlerFunctionType = typename EventDispatcher::HandlerFunctionType;

    static EventMaskType const EVENT_MASK = static_cast<EventMaskType>(1U << Event);

    explicit EventPolicy(EventDispatcher& eventDispatcher);

    void setEventHandler(HandlerFunctionType handlerFunction);
    void removeEventHandler();

    void setEvent();

private:
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
    _eventDispatcher.setEvents(EVENT_MASK);
}

} // namespace async
