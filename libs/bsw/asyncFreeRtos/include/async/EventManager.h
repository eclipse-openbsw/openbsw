// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#pragma once

#include "async/EventDispatcher.h"
#include "async/Types.h"

namespace async
{
template<size_t EventCount>
class EventManager : public EventDispatcher<EventCount, LockType>
{
public:
    EventManager();

    void init(TaskHandle_t handle);

    void setEvents(EventMaskType eventMask);

    static EventMaskType waitEvents();
    EventMaskType peekEvents();

private:
    static EventMaskType const WAIT_ALL_MASK = static_cast<EventMaskType>(0U - 1U);

    TaskHandle_t _handle;
};

/**
 * Inline implementations.
 */
template<size_t EventCount>
inline EventManager<EventCount>::EventManager() : _handle(0L)
{}

template<size_t EventCount>
inline void EventManager<EventCount>::init(TaskHandle_t handle)
{
    _handle = handle;
}

template<size_t EventCount>
inline void EventManager<EventCount>::setEvents(EventMaskType eventMask)
{
    BaseType_t* const higherPriorityHasWoken = OsBinding::getHigherPriorityHasWoken();
    if (higherPriorityHasWoken != 0L)
    {
        ::xTaskNotifyFromISR(_handle, eventMask, eSetBits, higherPriorityHasWoken);
    }
    else
    {
        ::xTaskNotify(_handle, eventMask, eSetBits);
    }
}

template<size_t EventCount>
inline EventMaskType EventManager<EventCount>::waitEvents()
{
    uint32_t eventMask = 0U;
    ::xTaskNotifyWait(0U, WAIT_ALL_MASK, &eventMask, 1000U);
    return eventMask;
}

template<size_t EventCount>
inline EventMaskType EventManager<EventCount>::peekEvents()
{
    uint32_t eventMask = 0U;
    ::xTaskNotifyWait(0U, 0U, &eventMask, 0U);
    return eventMask;
}

} // namespace async

