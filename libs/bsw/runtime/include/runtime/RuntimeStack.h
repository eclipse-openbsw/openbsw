// Copyright 2024 Accenture.

/**
 * \ingroup runtime
 */
#pragma once

#include <cstdint>

namespace runtime
{
template<class Entry>
class RuntimeStack
{
public:
    RuntimeStack() = default;

    Entry* getTopEntry() const { return _topEntry; }

    void pushEntry(Entry& entry, uint32_t const now)
    {
        if (!entry.isUsed())
        {
            entry.push(now, _topEntry);
            _topEntry = &entry;
        }
    }

    void popEntry(uint32_t const now)
    {
        if (_topEntry != nullptr)
        {
            _topEntry = _topEntry->pop(now);
        }
    }

    void popEntry(Entry& entry, uint32_t const now)
    {
        if (_topEntry == &entry)
        {
            _topEntry = entry.pop(now);
        }
    }

    void suspend(uint32_t const now) { _suspendTimestamp = now; }

    void resume(uint32_t const now) { addSuspendedTime(now - _suspendTimestamp); }

    void addSuspendedTime(uint32_t const suspendedTime)
    {
        if (_topEntry != nullptr)
        {
            _topEntry->addSuspendedTime(suspendedTime);
        }
    }

private:
    Entry* _topEntry           = nullptr;
    uint32_t _suspendTimestamp = 0U;
};

} // namespace runtime

