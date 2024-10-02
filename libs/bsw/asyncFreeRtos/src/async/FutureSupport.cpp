// Copyright 2024 Accenture.

#include "async/FutureSupport.h"

#include <async/AsyncBinding.h>

namespace async
{
static EventBits_t const FUTURE_SUPPORT_BITS_TO_WAIT = 0x01U;

FutureSupport::FutureSupport(ContextType const context) : _context(context)
{
    _eventGroupHandle = xEventGroupCreateStatic(&_eventGroup);
}

void FutureSupport::wait()
{
    TickType_t const waitEventsTickCount = (AsyncBinding::AdapterType::getCurrentTaskContext()
                                            == AsyncBinding::AdapterType::TASK_IDLE)
                                               ? 0U
                                               : AsyncBinding::WAIT_EVENTS_TICK_COUNT;
    while (true)
    {
        if (xEventGroupWaitBits(
                _eventGroupHandle, FUTURE_SUPPORT_BITS_TO_WAIT, pdTRUE, pdTRUE, waitEventsTickCount)
            == FUTURE_SUPPORT_BITS_TO_WAIT)
        {
            return;
        }
    }
}

void FutureSupport::notify()
{
    BaseType_t* const higherPriorityTaskWoken
        = AsyncBinding::AdapterType::getHigherPriorityTaskWoken();
    if (higherPriorityTaskWoken == nullptr)
    {
        (void)xEventGroupSetBits(_eventGroupHandle, FUTURE_SUPPORT_BITS_TO_WAIT);
    }
    else
    {
        (void)xEventGroupSetBitsFromISR(
            _eventGroupHandle, FUTURE_SUPPORT_BITS_TO_WAIT, higherPriorityTaskWoken);
    }
}

void FutureSupport::assertTaskContext() { estd_assert(verifyTaskContext()); }

bool FutureSupport::verifyTaskContext()
{
    return _context == AsyncBinding::AdapterType::getCurrentTaskContext();
}

} // namespace async
