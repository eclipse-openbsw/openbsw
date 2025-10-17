// Copyright 2025 Accenture.

#include "async/FutureSupport.h"

#include <etl/error_handler.h>
#include <etl/to_string.h>

#include <async/AsyncBinding.h>

namespace async
{
static uint32_t const FUTURE_SUPPORT_BITS_TO_WAIT = 0x01U;

FutureSupport::FutureSupport(ContextType const context) : _context(context), _eventName("future")
{
    ::etl::to_string(static_cast<uint8_t>(context), _eventName, true);
    tx_event_flags_create(&_eventObject, const_cast<CHAR*>(_eventName.c_str()));
}

void FutureSupport::wait()
{
    ULONG const waitEventsTickCount = (AsyncBinding::AdapterType::getCurrentTaskContext()
                                       == AsyncBinding::AdapterType::TASK_IDLE)
                                          ? 0U
                                          : AsyncBinding::WAIT_EVENTS_TICK_COUNT;
    while (true)
    {
        ULONG events;

        tx_event_flags_get(
            &_eventObject,
            FUTURE_SUPPORT_BITS_TO_WAIT,
            TX_AND_CLEAR, // wait for all bits, clear if successful
            &events,
            waitEventsTickCount);

        if (events == FUTURE_SUPPORT_BITS_TO_WAIT)
        {
            return;
        }
    }
}

void FutureSupport::notify()
{
    tx_event_flags_set(
        &_eventObject,
        FUTURE_SUPPORT_BITS_TO_WAIT,
        TX_OR // set eventMask bits in addition (OR)
    );
}

void FutureSupport::assertTaskContext()
{
    ETL_ASSERT(verifyTaskContext(), ETL_ERROR_GENERIC("TaskContext must be verified"));
}

bool FutureSupport::verifyTaskContext()
{
    return _context == AsyncBinding::AdapterType::getCurrentTaskContext();
}

} // namespace async
