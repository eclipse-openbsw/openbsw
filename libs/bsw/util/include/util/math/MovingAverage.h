// Copyright 2024 Accenture.

#ifndef GUARD_515D84D9_DA5F_4338_987F_C2497E2B8C25
#define GUARD_515D84D9_DA5F_4338_987F_C2497E2B8C25

#include <estd/tiny_ring.h>

#include <cstddef>

namespace util
{
namespace math
{

/**
 * Based on the tiny_ring container implementation of the moving average algorithm.
 *
 * This class should be initialized by the threshold value in the constructor.
 * All elements of the class will be equal to this value after the initialization.
 * First added element will move the average result below or above the threshold.
 *
 * Maximum number of stored elements is 256. This is a limitation of
 * the tiny_ring container.
 *
 */
template<class T, uint8_t N>
class MovingAverage
{
public:
    explicit MovingAverage(T const initial) : _values(), _sum(initial * N)
    {
        while (!_values.full())
        {
            _values.push_back(initial);
        }
    }

    void add(T const element)
    {
        _sum -= _values.front();
        _values.push_back(element);
        _sum += element;
    }

    T get() const { return _sum / static_cast<T>(N); }

private:
    ::estd::tiny_ring<T, N> _values;
    T _sum;
};

} // namespace math
} // namespace util

#endif /* GUARD_515D84D9_DA5F_4338_987F_C2497E2B8C25 */
