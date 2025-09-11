// Copyright 2025 Accenture.

/**
 * \ingroup async
 */
#pragma once

#include "async/EventDispatcher.h"
#include "async/Types.h"
#include "tx_api.h"

namespace async
{
template<size_t EventCount>
class EventManager : public EventDispatcher<EventCount, LockType>
{
public:
    EventManager();

    void init(TX_THREAD* handle);

    void setEvents(EventMaskType eventMask);

    static EventMaskType waitEvents();
    EventMaskType peekEvents();

private:
    static EventMaskType const WAIT_ALL_MASK = static_cast<EventMaskType>(0U - 1U);

    TX_THREAD* _handle;
    TX_EVENT_FLAGS_GROUP _eventObject;
};

/**
 * Inline implementations.
 */
template<size_t EventCount>
inline EventManager<EventCount>::EventManager() : _handle(nullptr)
{}

template<size_t EventCount>
inline void EventManager<EventCount>::init(TX_THREAD& handle)
{
    _handle = &handle;

    UINT status = tx_event_flags_create(
        &_eventObject,
        const_cast<CHAR*>(handle.tx_thread_name) // use same name as for the task
    );

    estd_assert(status == TX_SUCCESS);
}

template<size_t EventCount>
inline void EventManager<EventCount>::setEvents(EventMaskType eventMask)
{
    tx_event_flags_set(
        &_eventObject,
        eventMask,
        TX_OR // set eventMask bits in addition (OR)
    );
}

template<size_t EventCount>
inline EventMaskType EventManager<EventCount>::waitEvents()
{
    ULONG events = 0;
    tx_event_flags_get(
        &_eventObject,
        WAIT_ALL_MASK,
        TX_OR_CLEAR, // wait for any event, clear active events
        &events,
        TX_WAIT_FOREVER);

    return events;
}

template<size_t EventCount>
inline EventMaskType EventManager<EventCount>::peekEvents()
{
    ULONG events = 0;
    tx_event_flags_get(
        &_eventObject,
        WAIT_ALL_MASK,
        TX_OR_CLEAR, // wait for any event, clear active events
        &events,
        TX_NO_WAIT);

    return events;
}

} // namespace async
