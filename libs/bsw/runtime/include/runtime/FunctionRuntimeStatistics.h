// Copyright 2024 Accenture.

/**
 * \ingroup runtime
 */
#pragma once

#include "runtime/RuntimeStatistics.h"

namespace runtime
{
class FunctionRuntimeStatistics : public RuntimeStatistics
{
public:
    FunctionRuntimeStatistics() = default;

    void addRun(uint32_t const startTimestamp, uint32_t const runtime)
    {
        uint32_t const totalRunCount = getTotalRunCount();
        if (totalRunCount > 0U)
        {
            uint32_t const jitter = startTimestamp - _prevTimestamp;
            if (totalRunCount == 1U)
            {
                _minJitter = jitter;
                _maxJitter = jitter;
            }
            else
            {
                if (jitter < _minJitter)
                {
                    _minJitter = jitter;
                }
                if (jitter > _maxJitter)
                {
                    _maxJitter = jitter;
                }
            }
        }
        _prevTimestamp = startTimestamp;
        RuntimeStatistics::addRun(runtime);
    }

    void addRun(uint32_t const startTimestamp, uint32_t const runtime, uint32_t const)
    {
        addRun(startTimestamp, runtime);
    }

    void reset()
    {
        RuntimeStatistics::reset();
        _minJitter     = 0U;
        _maxJitter     = 0U;
        _prevTimestamp = 0U;
    }

    uint32_t getMinJitter() const { return _minJitter; }

    uint32_t getMaxJitter() const { return _maxJitter; }

private:
    uint32_t _minJitter     = 0U;
    uint32_t _maxJitter     = 0U;
    uint32_t _prevTimestamp = 0U;
};

} // namespace runtime
