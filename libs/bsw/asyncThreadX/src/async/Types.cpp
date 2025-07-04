// Copyright 2025 Accenture.

#include "async/AsyncBinding.h"

namespace async
{
TimeoutType::TimeoutType() : _runnable(nullptr), _context(0) {}

void TimeoutType::cancel() { AsyncBindingType::AdapterType::cancel(*this); }

void TimeoutType::expired()
{
    RunnableType* const runnable = _runnable;
    if (runnable != nullptr)
    {
        runnable->execute();
    }
}

} // namespace async
