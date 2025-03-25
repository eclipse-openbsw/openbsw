// Copyright 2024 Accenture.

#pragma once

#include "estd/slice.h"

#include <platform/estdint.h>

#include <new>

namespace estd
{
template<class T>
class ring
{
    explicit ring(uint32_t const size) : _length(size), _count(0U), _read(0U) {}

public:
    /**
     * Construct a new ring
     */
    static ring* make(uint32_t const size, slice<uint8_t>& s)
    {
        ring* const r = new (s.data()) ring(size);
        s             = s.offset(sizeof(ring) + sizeof(T) * size);
        return r;
    }

    /**
     * Get a copy of the oldest element in the ring.
     */
    T front() const { return data()[_read]; }

    /**
     * Get a copy of the element at the index
     */
    T at(uint32_t const position) const
    {
        return data()[static_cast<size_t>((_read + position) % _length)];
    }

    /**
     * Return true if there are no elements in the ring. False otherwise.
     */
    bool empty() const { return _count == 0U; }

    /**
     * Return true if there are exactly _length elements in the ring. False otherwise.
     */
    bool full() const { return _count == _length; }

    /**
     * Remove the oldest element from the ring. Calling this on an empty ring is
     * undefined behaviour.
     */
    void pop_front()
    {
        --_count;
        _read = (_read + 1U) % _length;
    }

    /**
     * Add an element to the ring.
     * Adding to a full ring will overwrite the oldest element.
     */
    void push_back(T const value)
    {
        data()[(_read + _count) % _length] = value;

        if (full())
        {
            _read = (_read + 1U) % _length;
        }
        else
        {
            ++_count;
        }
    }

    /**
     * Return underlying array as slice
     */
    ::estd::slice<T> data() const
    {
        return slice<T>::from_pointer(
            const_cast<T*>(reinterpret_cast<T const*>(this + 1)), _length);
    }

    uint32_t length() const { return _length; }

    uint32_t used() const { return _count; }

private:
    uint32_t _length;
    uint32_t _count;
    uint32_t _read;
};

} // namespace estd
