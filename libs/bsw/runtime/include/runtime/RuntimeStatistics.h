// Copyright 2024 Accenture.

/**
 * \ingroup runtime
 */
#pragma once

#include <cstdint>

namespace runtime
{
class RuntimeStatistics
{
public:
    RuntimeStatistics() = default;

    void addRun(uint32_t const runtime)
    {
        _totalRuntime += runtime;
        if (_totalRunCount == 0U)
        {
            _minRuntime = runtime;
            _maxRuntime = runtime;
        }
        else
        {
            if (_minRuntime > runtime)
            {
                _minRuntime = runtime;
            }
            if (_maxRuntime < runtime)
            {
                _maxRuntime = runtime;
            }
        }
        ++_totalRunCount;
    }

    void addRun(uint32_t const, uint32_t const runtime, uint32_t const) { addRun(runtime); }

    void reset()
    {
        _totalRuntime  = 0U;
        _totalRunCount = 0U;
        _minRuntime    = 0U;
        _maxRuntime    = 0U;
    }

    uint32_t getTotalRuntime() const { return _totalRuntime; }

    uint32_t getTotalRunCount() const { return _totalRunCount; }

    uint32_t getMinRuntime() const { return _minRuntime; }

    uint32_t getMaxRuntime() const { return _maxRuntime; }

    uint32_t getAverageRuntime() const
    {
        return (_totalRunCount != 0U) ? (_totalRuntime / _totalRunCount) : 0U;
    }

private:
    uint32_t _totalRuntime  = 0;
    uint32_t _totalRunCount = 0;
    uint32_t _minRuntime    = 0;
    uint32_t _maxRuntime    = 0;
};

} // namespace runtime

