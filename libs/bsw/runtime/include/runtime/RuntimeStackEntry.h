// Copyright 2024 Accenture.

/**
 * \ingroup runtime
 */
#pragma once

#include <cstdint>

namespace runtime
{
namespace internal
{
template<bool CutOut>
struct CutOutPolicy
{};

template<>
struct CutOutPolicy<false>
{
    static uint32_t getDelta(uint32_t const /*runtime*/) { return 0U; }
};

template<>
struct CutOutPolicy<true>
{
    static uint32_t getDelta(uint32_t const runtime) { return runtime; }
};

} // namespace internal

template<class Entry, bool CutOut>
class RuntimeStackEntry
{
public:
    RuntimeStackEntry() = default;

    bool isUsed() const { return _prevEntry != reinterpret_cast<Entry*>(1U); }

    void push(uint32_t const now, Entry* const prevEntry)
    {
        if (!isUsed())
        {
            _prevEntry      = prevEntry;
            _startTimestamp = now;
            _suspendedTime  = 0U;
        }
    }

    Entry* pop(uint32_t const now)
    {
        uint32_t const thisRuntime = now - _startTimestamp - _suspendedTime;

        static_cast<Entry*>(this)->addRun(_startTimestamp, thisRuntime, _suspendedTime);
        Entry* const prevEntry = _prevEntry;
        if (prevEntry != nullptr)
        {
            prevEntry->addSuspendedTime(
                _suspendedTime + internal::CutOutPolicy<CutOut>::getDelta(thisRuntime));
        }
        _prevEntry = reinterpret_cast<Entry*>(1U);
        return prevEntry;
    }

    void addSuspendedTime(uint32_t const suspendedTime) { _suspendedTime += suspendedTime; }

private:
    Entry* _prevEntry        = reinterpret_cast<Entry*>(1U);
    uint32_t _startTimestamp = 0U;
    uint32_t _suspendedTime  = 0U;
};

} // namespace runtime
