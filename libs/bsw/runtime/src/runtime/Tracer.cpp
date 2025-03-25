// Copyright 2025 Accenture.

#include "runtime/Tracer.h"

#include "bsp/timer/SystemTimer.h"

namespace runtime
{

uint32_t Tracer::_ramTraces[TRACING_BUFFER_WORD_SIZE];
bool Tracer::_running        = false;
uint32_t Tracer::_pos        = 0;
uint32_t Tracer::_prevCycles = 0;

void Tracer::init()
{
    _running    = false;
    _pos        = 0;
    _prevCycles = 0;
    for (size_t i = 0; i < sizeof(_ramTraces) / sizeof(uint32_t); i++)
    {
        _ramTraces[i] = 0;
    }
    // write ticks per second to first trace buffer word
    _ramTraces[_pos++] = getFastTicksPerSecond();
}

void Tracer::start() { _running = true; }

void Tracer::stop() { _running = false; }

void Tracer::traceThreadSwitchedIn(uint8_t const taskIdx)
{
    traceEvent(EVENT_THREAD_SWITCHED_IN, taskIdx);
}

void Tracer::traceThreadSwitchedOut(uint8_t const taskIdx)
{
    traceEvent(EVENT_THREAD_SWITCHED_OUT, taskIdx);
}

void Tracer::traceIsrEnter(uint8_t const isrIdx) { traceEvent(EVENT_ISR_ENTER, isrIdx); }

void Tracer::traceIsrExit(uint8_t const isrIdx) { traceEvent(EVENT_ISR_EXIT, isrIdx); }

void Tracer::traceUser(uint8_t const usrIdx) { traceEvent(EVENT_USER, usrIdx); }

inline bool Tracer::bufferFull()
{
    // buffer is full if we can't store another extended frame (2 words)
    return _pos >= TRACING_BUFFER_WORD_SIZE - 1;
}

/**
 *  Store a trace event with id argument.
 *
 *  There are two types a frames: normal frames and extended frames.
 *  Each frame stores the relative time since the last tracing event.
 *  If that time is too long for a normal frame, an extended frame will be used
 *  instead and the time stored is the absolute instead of the relative time.
 *
 *  .     .     .     .     .     .     .     .     .
 *  | ctr | arg | rel cycles|                               normal frame (4 bytes)
 *  | ctr | arg | padding   |       abs cycles      |       extended frame (8 bytes)
 *
 *  ctr: control byte, see below
 *  arg: argument for event, e.g. task ID, isr ID
 *  rel cycles: relative HW cycles since last frame
 *  abs time: absolute HW cycles
 *
 *  Control Byte:
 *
 *  .     .     .     .     .     .     .     .     .
 *  | ext |       evt       |     rel cycles high   |       control byte (8 bits)
 *
 *  ext: if set, this is an extended frame
 *  evt: event id (0-7)
 *  rel cycles high: top 4 bits of rel cycles, making rel time a 20 bit value
 */
void Tracer::traceEvent(Event const& event, uint8_t const& id)
{
    if (!_running || bufferFull())
    {
        return;
    }

    uint32_t cycles = getFastTicks();
    uint32_t diff;

    // Calculate time since last trace event, taking wrapping into account.
    // If more time has passed than what fits into a uint32_t this will be incorrect.
    // The assumption is that tracing events are rather frequent (e.g. task switches)
    if (cycles > _prevCycles)
    {
        diff = cycles - _prevCycles;
    }
    else
    {
        diff = 0xffffffff - _prevCycles + cycles + 1;
    }

    if (_prevCycles != 0 && diff <= RELATIVE_CYCLES_MAX)
    {
        // normal frame
        uint8_t ctrl = (event << 4) & 0x70;
        if (diff > 0xffff)
        {
            ctrl |= (diff >> 16) & 0xf;
        }
        _ramTraces[_pos++] = (ctrl << 24) | (id << 16) | (diff & 0xffff);
    }
    else
    {
        // extended frame
        uint8_t ctrl       = 0x80 | ((event << 4) & 0x70);
        _ramTraces[_pos++] = (ctrl << 24) | (id << 16);
        _ramTraces[_pos++] = cycles;
    }
    _prevCycles = cycles;
}

} // namespace runtime
