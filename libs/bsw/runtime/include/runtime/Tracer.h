// Copyright 2025 Accenture.

#pragma once

#include "FreeRTOSConfig.h"

#include <platform/estdint.h>

namespace runtime
{
#ifndef TRACING_BUFFER_SIZE
#define TRACING_BUFFER_SIZE 4096
#endif

class Tracer
{
public:
    Tracer();

    static void init();
    static void start();
    static void stop();

    static void traceThreadSwitchedOut(uint8_t const taskIdx);
    static void traceThreadSwitchedIn(uint8_t const taskIdx);
    static void traceIsrEnter(uint8_t const isrIdx);
    static void traceIsrExit(uint8_t const isrIdx);
    static void traceUser(uint8_t const usrIdx);

    static bool bufferFull();

private:
    enum Event
    {
        EVENT_THREAD_SWITCHED_OUT = 0x0,
        EVENT_THREAD_SWITCHED_IN  = 0x1,
        EVENT_ISR_ENTER           = 0x2,
        EVENT_ISR_EXIT            = 0x3,
        EVENT_USER                = 0x4,
    };

    static constexpr uint32_t TRACING_BUFFER_WORD_SIZE = TRACING_BUFFER_SIZE / sizeof(uint32_t);

    static constexpr uint32_t RELATIVE_CYCLES_WIDTH = 20;
    static constexpr uint32_t RELATIVE_CYCLES_MAX   = (1 << RELATIVE_CYCLES_WIDTH) - 1;

    static void traceEvent(Event const& event, uint8_t const& id);

private:
    static uint32_t _ramTraces[TRACING_BUFFER_WORD_SIZE];
    static bool _running;
    static uint32_t _pos;
    static uint32_t _prevCycles;
};

} // namespace runtime
