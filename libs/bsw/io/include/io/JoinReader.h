// Copyright 2024 Accenture.

#pragma once

#include "io/IReader.h"

#include <etl/array.h>
#include <etl/span.h>
#include <util/estd/assert.h>

namespace io
{
/**
 * Class which wraps (joins) N readers into one single reader interface.
 *
 * The reader will return from each of the readers in sequence.
 *
 * [TPARAMS_BEGIN]
 * \tparam N Number of IReaders this JoinReader wraps.
 * [TPARAMS_END]
 */
template<size_t N>
class JoinReader : public IReader
{
public:
    // [PUBLIC_API_BEGIN]
    /**
     * Constructs a JoinReader from an array of IReader sources.
     *
     * \param sources   Slice of pointers (must not be nullptr) to IReader.
     *
     * \assert sources[0..N-1] != nullptr
     * \assert sources[0]->maxSize() == sources[1]->maxSize() ... == sources[N-1]->maxSize()
     */
    explicit JoinReader(::etl::span<IReader*, N> const& sources);

    /**
     * \see ::io::IReader::maxSize()
     */
    size_t maxSize() const override;

    /**
     * Returns a slice to the next piece of available data.
     *
     * The strategy used is a form of round-robin, where one element is returned from each queue
     * in turn.
     * Note that the same element will keep getting returned until release() is called.
     *
     * \return - Slice of bytes if one of the underlying queues was not empty
     *         - Empty slice otherwise
     */
    ::etl::span<uint8_t> peek() const override;

    /**
     * \see ::io::IReader::release()
     */
    void release() override;
    // [PUBLIC_API_END]

    /**
     * The number of peeked messages released from each IReader.
     */
    ::etl::array<size_t, N> stats;

private:
    ::etl::span<IReader*, N> _sources;
    mutable size_t _currentPosition;
    mutable bool _hasPeeked = false;
};

template<size_t N>
JoinReader<N>::JoinReader(::etl::span<IReader*, N> const& sources)
: stats(), _sources(sources), _currentPosition(0)
{
    estd_assert(_sources[0] != nullptr);
    auto const max = _sources[0]->maxSize();
    for (size_t i = 1; i < N; i++)
    {
        estd_assert(_sources[i] != nullptr);
        auto const maxi = _sources[i]->maxSize();
        estd_assert(maxi == max);
    }
}

template<size_t N>
inline size_t JoinReader<N>::maxSize() const
{
    return _sources[0]->maxSize();
}

template<size_t N>
::etl::span<uint8_t> JoinReader<N>::peek() const
{
    for (size_t i = _currentPosition; i < (N + _currentPosition); i++)
    {
        size_t const real_i = i % N;
        auto const b        = _sources[real_i]->peek();
        if (b.size() > 0)
        {
            _currentPosition = real_i;
            _hasPeeked       = true;
            return b;
        }
    }
    return {};
}

template<size_t N>
void JoinReader<N>::release()
{
    if (_hasPeeked)
    {
        stats[_currentPosition] += 1;
    }
    _sources[_currentPosition]->release();
    _currentPosition++;
    _currentPosition %= N;
    _hasPeeked = false;
}

} // namespace io
