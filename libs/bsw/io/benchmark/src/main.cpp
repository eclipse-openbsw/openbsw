// Copyright 2024 Accenture.

#include <benchmark/benchmark.h>
#include <io/MemoryQueue.h>

namespace
{
constexpr uint8_t MAX_SIZE   = 12;
constexpr uint8_t SMALL_SIZE = 3;
using Queue                  = ::io::MemoryQueue<1024 * 10, MAX_SIZE>;
using Writer                 = ::io::MemoryQueueWriter<Queue>;
using Reader                 = ::io::MemoryQueueReader<Queue>;
} // namespace

/**
 * Benchmarks the use case of allocating a MAX_SIZE slice and then trimming to a smaller size
 * before committing through a re-allocation.
 */
void BM_fill_empty_queue_reallocate(benchmark::State& state)
{
    Queue q;
    Writer w(q);
    Reader r(q);

    while (state.KeepRunning())
    {
        // fill queue
        auto s = w.allocate(MAX_SIZE);
        while (s.size() > 0)
        {
            // reallocate to smaller size
            s    = w.allocate(SMALL_SIZE);
            s[0] = 0xAAU;
            s[1] = 0xBBU;
            s[2] = 0xCCU;
            // commit
            w.commit();
            // full allocation
            s = w.allocate(MAX_SIZE);
        }
        // empty queue
        s = r.peek();
        while (s.size() > 0)
        {
            r.release();
            s = r.peek();
        }
    }
}

BENCHMARK(BM_fill_empty_queue_reallocate);
