// Copyright 2024 Accenture.

#pragma once

#include "estd/array.h"
#include "estd/slice.h"

#include <platform/estdint.h>

namespace estd
{
/**
 * A space optimized implementation for small ring buffers.
 *
 * This ring buffer implementation has very low memory overhead (2 bytes)
 * but is limited to rings of a maximum size of 255 elements.
 * Also writing to a full ring buffer will overwrite old elements without notice.
 */
template<class T, size_t N>
class tiny_ring
{
    // Index variable is uint8_t which will overflow at 255.
    static_assert(N < 256, "");

public:
    /**
     * Construct a new tiny_ring
     */
    tiny_ring();

    /**
     * Get a copy of the oldest element in the ring.
     */
    T front() const;

    /**
     * Return true if there are no elements in the ring. False otherwise.
     */
    bool empty() const;

    /**
     * Return true if there are exactly N elements in the ring. False otherwise.
     */
    bool full() const;

    /**
     * Remove the oldest element from the ring. Calling this on an empty ring is
     * undefined behaviour.
     */
    void pop_front();

    /**
     * Add an element to the ring.
     * Adding to a full ring will overwrite the oldest element.
     */
    void push_back(T value);

    slice<T, N> data() { return _data; }

private:
    array<T, N> _data;
    uint8_t _count;
    uint8_t _read;
};

template<class T, size_t N>
tiny_ring<T, N>::tiny_ring() : _data(), _count(0U), _read(0U)
{}

template<class T, size_t N>
bool tiny_ring<T, N>::empty() const
{
    return _count == 0U;
}

template<class T, size_t N>
bool tiny_ring<T, N>::full() const
{
    return _count == N;
}

template<class T, size_t N>
void tiny_ring<T, N>::push_back(T const value)
{
    _data[static_cast<size_t>((_read + _count) % N)] = value;

    if (full())
    {
        _read = static_cast<uint8_t>((static_cast<size_t>(1) + _read) % N);
    }
    else
    {
        ++_count;
    }
}

template<class T, size_t N>
void tiny_ring<T, N>::pop_front()
{
    --_count;
    _read = static_cast<uint8_t>((static_cast<size_t>(1) + _read) % N);
}

template<class T, size_t N>
T tiny_ring<T, N>::front() const
{
    return _data[static_cast<size_t>(_read)];
}

} // namespace estd

