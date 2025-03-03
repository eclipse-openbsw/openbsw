// Copyright 2024 Accenture.

/**
 * Contains estd::bitset.
 * \file
 * \ingroup estl_containers
 */
#ifndef GUARD_0FFBC651_ED5E_436C_BAA9_FCE7EAABE3BA
#define GUARD_0FFBC651_ED5E_436C_BAA9_FCE7EAABE3BA

#include "estd/array.h"
#include "estd/assert.h"
#include "estd/memory.h"
#include "estd/slice.h"

#include <platform/estdint.h>

#include <algorithm>
#include <limits>
#include <type_traits>

namespace estd
{
namespace internal
{
/**
 * Helper class to calculate mask for the last element of an array based bitset.
 * \tparam Size    Size of bitset
 */
template<size_t N>
struct mask_helper
{
    static uint32_t const value = (0xFFFFFFFFU >> (32U - N));
};

template<>
struct mask_helper<0>
{
    static uint32_t const value = 0xFFFFFFFFU;
};

/**
 * Bitset implementation using an integral type as underlying storage.
 * \tparam T    Underlying integral type
 * \tparam Size Number of bits
 */
template<class T, size_t Size>
class integral_bitset
{
    static_assert(Size <= (sizeof(T) * 8U), "Size is not supported for this T");

    static T const mask = static_cast<T>(~static_cast<T>(0)) >> ((sizeof(T) * 8U) - Size);

public:
    using value_type = T;

    /**
     * Proxy object to reference a single bit inside the bitset.
     * This allows both reading as well as writing of the referenced bit.
     */
    class reference
    {
    public:
        /**
         * Write the truth value represented by "rhs" to the bit.
         */
        reference& operator=(bool const rhs)
        {
            _bitset.set(_pos, rhs);
            return *this;
        }

        /**
         * Write the truth value represented by "rhs" to the bit.
         */
        reference& operator=(reference const& rhs)
        {
            _bitset.set(_pos, rhs);
            return *this;
        }

        /**
         * Converts referenced bit to a boolean.
         */
        operator bool() const { return _bitset.test(_pos); }

        /**
         * Returns inverted value of referenced bit.
         */
        bool operator~() const { return !(this->operator bool()); }

        /**
         * Flips the referenced bit.
         */
        reference& flip()
        {
            _bitset.set(_pos, !_bitset.test(_pos));
            return *this;
        }

    private:
        friend class integral_bitset;

        reference(integral_bitset& refed_bitset, size_t const pos)
        : _pos(pos), _bitset(refed_bitset)
        {}

        size_t _pos;
        integral_bitset& _bitset;
    };

    integral_bitset() : _value() {}

    integral_bitset(value_type const initial_value) : _value(initial_value & mask) {}

    explicit integral_bitset(::estd::slice<value_type const, 1> const initial_value)
    : _value(initial_value[0U] & mask)
    {}

    void assign(::estd::slice<value_type const, 1> const value) { _value = value[0] & mask; }

    void from_bytes(::estd::slice<uint8_t const> values)
    {
        reset();
        auto const slice = ::estd::reinterpret_as<uint8_t>(::estd::memory::as_slice(&_value, 1));
        if (slice.size() < values.size())
        {
            values.trim(slice.size());
        }
        (void)::estd::memory::copy(slice, values);
    }

    value_type value() const { return _value & mask; }

    ::estd::slice<value_type const, 1> to_slice() const
    {
        return ::estd::slice<value_type const, 1>::from_pointer(&_value);
    }

    void set() { _value = mask; }

    void set(size_t const pos, bool const value)
    {
        estd_assert(pos < Size);

        value_type tmp = 1U;
        tmp <<= static_cast<value_type>(pos);
        if (value)
        {
            _value |= static_cast<value_type>(tmp);
        }
        else
        {
            _value &= static_cast<value_type>(~tmp);
        }
    }

    void reset() { _value = 0U; }

    void reset(size_t const pos)
    {
        value_type tmp = 1U;
        tmp <<= static_cast<value_type>(pos);
        _value &= static_cast<value_type>(~tmp);
    }

    bool test(size_t const pos) const
    {
        value_type tmp = 1U;
        tmp <<= static_cast<value_type>(pos);
        return ((_value & tmp) != 0U);
    }

    bool all() const { return (_value == mask); }

    bool any() const { return (_value != 0U); }

    reference make_ref(size_t const pos) { return reference(*this, pos); }

    void flip() { _value = (static_cast<value_type>(~_value) & mask); }

    /* Implementation of SWAR bitcount algorithm based on:
    https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel */
    size_t count() const
    {
        constexpr value_type m1 = std::numeric_limits<value_type>::max() / 3U; // binary: 0101...
        constexpr value_type m2
            = std::numeric_limits<value_type>::max() / 15U * 3U; // binary: 00110011..
        constexpr value_type m4
            = std::numeric_limits<value_type>::max() / 255U * 15U; // binary:  00001111 ...
        constexpr value_type h01 = std::numeric_limits<value_type>::max()
                                   / 255U; // the sum of 256 to the power of 0,1,2,3...
        value_type tmp = _value;
        tmp            = tmp - ((tmp >> 1U) & m1);
        tmp            = (tmp & m2) + ((tmp >> 2U) & m2);
        tmp            = (tmp + (tmp >> 4U)) & m4;
        tmp            = static_cast<value_type>(tmp * h01) >> ((sizeof(value_type) - 1U) * 8U);
        return tmp;
    }

    void operator&=(integral_bitset const& rhs) { _value &= rhs._value; }

    void operator|=(integral_bitset const& rhs) { _value |= rhs._value; }

    void operator^=(integral_bitset const& rhs) { _value ^= rhs._value; }

    void operator>>=(size_t const pos) { _value >>= pos; }

    void operator<<=(size_t const pos) { _value <<= pos; }

    bool operator==(integral_bitset const& rhs) const { return _value == rhs._value; }

private:
    value_type _value;
};

template<>
class integral_bitset<uint8_t, 0>
{
public:
    using value_type = uint8_t;

    /**
     * Proxy object to reference a single bit inside the bitset of size 0.
     */
    class reference
    {
    public:
        reference(reference const&) = default;

        reference& operator=(bool) { return *this; }

        reference& operator=(reference const&) = default;

        operator bool() const { return false; }

        bool operator~() const { return true; }

        reference& flip() { return *this; }

    private:
        friend class integral_bitset;
        reference() = default;
    };

    integral_bitset() = default;

    integral_bitset(value_type) {}

    explicit integral_bitset(::estd::slice<value_type const, 0>) {}

    value_type value() const { return 0U; }

    void set() {}

    void set(size_t, bool) {}

    void reset() {}

    void reset(size_t) {}

    void assign(::estd::slice<value_type const, 1> const) {}

    void from_bytes(::estd::slice<uint8_t const> const&) {}

    bool test(size_t) const { return false; }

    bool all() const { return true; }

    bool any() const { return false; }

    reference make_ref(size_t) { return reference(); }

    void flip() {}

    void operator&=(integral_bitset const&) {}

    void operator|=(integral_bitset const&) {}

    void operator^=(integral_bitset const&) {}

    bool operator==(integral_bitset const&) const { return true; }
};

/**
 * Bitset implementation using an underlying array of uint32_t.
 * \tparam Size Number of bits
 * \tparam N    Size of underlying array
 */
template<size_t Size, size_t N>
class array_bitset
{
    static_assert(N > 0U, "N must be greater than zero!");
    using data_type                 = ::estd::array<uint32_t, N>;
    // mask for last element of underlying array
    static uint32_t const last_mask = mask_helper<(Size % 32)>::value;

public:
    using value_type = uint32_t;

    /**
     * Proxy object to reference a single bit inside the bitset.
     * This allows both reading as well as writing of the referenced bit.
     */
    class reference
    {
    public:
        /**
         * Write the truth value represented by "rhs" to the bit.
         */
        reference& operator=(bool const rhs)
        {
            _bitset.set(_pos, rhs);
            return *this;
        }

        /**
         * Write the truth value represented by "rhs" to the bit.
         */
        reference& operator=(reference const& rhs)
        {
            _bitset.set(_pos, rhs._bitset.test(rhs._pos));
            return *this;
        }

        operator bool() const { return _bitset.test(_pos); }

        bool operator~() const { return !_bitset.test(_pos); }

        reference& flip()
        {
            _bitset.set(_pos, !_bitset.test(_pos));
            return *this;
        }

    private:
        friend class array_bitset;

        reference(array_bitset& refed_bitset, size_t const pos) : _pos(pos), _bitset(refed_bitset)
        {}

        size_t _pos;
        array_bitset& _bitset;
    };

    array_bitset() : _data() {}

    array_bitset(value_type const initial_value) : _data() { _data[0U] = initial_value; }

    explicit array_bitset(::estd::slice<uint32_t const, N> const& initial_values)
    {
        assign(initial_values);
    }

    void assign(::estd::slice<uint32_t const, N> const& values)
    {
        for (size_t i = 0; i < N; ++i)
        {
            _data[i] = values[i];
        }
        _data[N - 1U] &= last_mask;
    }

    void from_bytes(::estd::slice<uint8_t const> values)
    {
        auto const slice = ::estd::slice<uint32_t>(_data).reinterpret_as<uint8_t>();
        if (slice.size() < values.size())
        {
            values.trim(slice.size());
        }
        else
        {
            ::estd::memory::set(slice.offset(values.size()), 0);
        }
        (void)::estd::memory::copy(slice, values);
        _data[N - 1U] &= last_mask;
    }

    value_type value() const
    {
        auto const e_itr = _data.end();
        auto const b_itr = _data.begin();
        // this is possible because N is always >= 1
        auto itr         = b_itr + 1; // Iterator arithmetic
        while (itr != e_itr)
        {
            estd_assert(*itr == 0U);
            ++itr;
        }
        return *b_itr;
    }

    ::estd::slice<uint32_t const, N> to_slice() const { return _data; }

    void set()
    {
        typename data_type::iterator const e_itr = _data.end();
        typename data_type::iterator itr         = _data.begin();
        while (itr != e_itr)
        {
            *itr = 0xFFFFFFFFU;
            ++itr;
        }
        _data[N - 1U] = last_mask;
    }

    void set(size_t pos, bool const value)
    {
        estd_assert(pos < Size);

        if (value)
        {
            // index in array
            size_t const i = pos / 32U;
            // bit position in element
            pos            = pos % 32U;
            value_type tmp = 1U;
            tmp <<= static_cast<value_type>(pos);
            _data[i] |= tmp;
        }
        else
        {
            reset(pos);
        }
    }

    void reset()
    {
        auto const e_itr = _data.end();
        std::fill(_data.begin(), e_itr, static_cast<value_type>(0U));
    }

    void reset(size_t pos)
    {
        // index in array
        size_t const i     = pos / 32U;
        // bit position in element
        pos                = pos % 32U;
        value_type const m = ((i == (N - 1U)) ? last_mask : 0xFFFFFFFFU);
        value_type tmp     = 1U;
        tmp <<= static_cast<value_type>(pos);
        _data[i] &= static_cast<value_type>(static_cast<value_type>(~tmp) & m);
    }

    bool test(size_t pos) const
    {
        size_t const i = pos / 32U;
        pos            = pos % 32U;
        value_type tmp = 1U;
        tmp <<= static_cast<value_type>(pos);
        return ((_data[i] & tmp) != 0U);
    }

    bool all() const
    {
        uint32_t const m = 0xFFFFFFFFU;
        auto const e_itr = _data.end() - 1U; // Iterator arithmetic
        auto itr         = _data.begin();
        while (itr != e_itr)
        {
            if ((*itr & m) != m)
            {
                return false;
            }
            ++itr;
        }
        return ((*itr & last_mask) == last_mask);
    }

    bool any() const
    {
        auto const e_itr = _data.end();
        for (auto itr = _data.begin(); itr != e_itr; ++itr)
        {
            if (*itr != 0U)
            {
                return true;
            }
        }
        return false;
    }

    reference make_ref(size_t const pos) { return reference(*this, pos); }

    void flip()
    {
        auto const e_itr = _data.end() - 1; // Iterator arithmetic
        auto itr         = _data.begin();
        while (itr != e_itr)
        {
            *itr = ~*itr;
            ++itr;
        }
        *itr = (~*itr & last_mask);
    }

    size_t count()
    {
        using intBitset = integral_bitset<value_type, sizeof(value_type) * 8>;
        size_t ones     = 0;
        auto const tmp  = ::estd::reinterpret_as<intBitset, value_type, N>(_data);
        for (size_t i = 0; i < _data.size(); ++i)
        {
            ones += tmp[i].count();
        }

        return ones;
    }

    void operator&=(array_bitset const& rhs)
    {
        for (size_t i = 0U; i < N; ++i)
        {
            _data[i] &= rhs._data[i];
        }
    }

    void operator|=(array_bitset const& rhs)
    {
        for (size_t i = 0U; i < N; ++i)
        {
            _data[i] |= rhs._data[i];
        }
    }

    void operator^=(array_bitset const& rhs)
    {
        for (size_t i = 0U; i < N; ++i)
        {
            _data[i] ^= rhs._data[i];
        }
    }

    void operator>>=(size_t const pos)
    {
        size_t const shift = pos % (sizeof(value_type) * 8);
        size_t const zeros = pos / (sizeof(value_type) * 8);

        for (size_t i = 0; i < _data.size(); ++i)
        {
            if (i < _data.size() - zeros - 1)
            {
                _data[i] = _data[i + zeros];
                _data[i] >>= shift;
                // In case shift == 0, just return 0 to prevent a shift of 32 bits
                // (technically undefined behavior)
                auto const left
                    = (shift == 0) ? 0 : _data[i + 1 + zeros] << (sizeof(value_type) * 8 - shift);
                _data[i] = left | _data[i];
            }
            else if (i == _data.size() - zeros - 1)
            {
                _data[i] >>= shift;
            }
            else
            {
                _data[i] = 0;
            }
        }
    }

    void operator<<=(size_t const pos)
    {
        size_t const shift = pos % (sizeof(value_type) * 8);
        size_t const zeros = pos / (sizeof(value_type) * 8);

        for (size_t i = _data.size(); i > 0; --i)
        {
            auto const idx = i - 1;
            if (idx > zeros)
            {
                _data[idx] = _data[idx - zeros];
                _data[idx] <<= shift;
                // In case shift == 0, just return 0 to prevent a shift of 32 bits (technically
                // undefined behavior)
                auto const right
                    = (shift == 0) ? 0
                                   : (_data[idx - 1 - zeros] >> (sizeof(value_type) * 8 - shift));
                _data[idx] = right | _data[idx];
            }
            else if (idx == zeros)
            {
                _data[idx] <<= shift;
            }
            else
            {
                _data[idx] = 0;
            }
        }
    }

    bool operator==(array_bitset const& rhs) const { return _data == rhs._data; }

private:
    data_type _data;
};

template<size_t Size>
struct type_for_size_helper
{
    using type            = void;
    // for any arbitrary length bitfield uint32_t will be internally used
    static size_t const N = ((Size * 8U) + 31U) / 32U;
};

template<>
struct type_for_size_helper<0U>
{
    using type            = uint8_t;
    static size_t const N = 0U;
};

template<>
struct type_for_size_helper<1U>
{
    using type            = uint8_t;
    static size_t const N = 1U;
};

template<>
struct type_for_size_helper<2U>
{
    using type            = uint16_t;
    static size_t const N = 1U;
};

template<>
struct type_for_size_helper<3U>
{
    using type            = uint32_t;
    static size_t const N = 1U;
};

template<>
struct type_for_size_helper<4U>
{
    using type            = uint32_t;
    static size_t const N = 1U;
};

template<>
struct type_for_size_helper<5U>
{
    using type            = uint64_t;
    static size_t const N = 1U;
};

template<>
struct type_for_size_helper<6U>
{
    using type            = uint64_t;
    static size_t const N = 1U;
};

template<>
struct type_for_size_helper<7U>
{
    using type            = uint64_t;
    static size_t const N = 1U;
};

template<>
struct type_for_size_helper<8U>
{
    using type            = uint64_t;
    static size_t const N = 1U;
};

template<size_t Size>
struct type_for_size : type_for_size_helper<(Size + 7U) / 8U>
{};

} // namespace internal

template<size_t Size>
class bitset
{
    class Unassignable
    {};

public:
    using data_type = typename std::conditional<
        (Size <= 64),
        ::estd::internal::
            integral_bitset<typename ::estd::internal::type_for_size<Size>::type, Size>,
        ::estd::internal::array_bitset<Size, ::estd::internal::type_for_size<Size>::N>>::type;

    static size_t const SIZE = Size;

    /**
     * Reference type for a single bit.
     */
    using reference  = typename data_type::reference;
    using value_type = typename data_type::value_type;
    using value_slice =
        typename ::estd::slice<value_type const, ::estd::internal::type_for_size<Size>::N>;

    /**
     * Constructs an empty bitset.
     */
    bitset() : _data() {}

    /**
     * Constructs a bitset from a given value.
     */
    bitset(typename std::conditional<
           (Size <= 64),
           typename ::estd::internal::type_for_size<Size>::type,
           Unassignable>::type const initial_value)
    : _data(initial_value)
    {}

    /**
     * Constructs a bitset from a given array of integral values.
     * \tparam N    Size of given array.
     */
    explicit bitset(value_slice const& initial_values) : _data(initial_values) {}

    /**
     * @brief
     * Assigns all bits to the same values as the bits in the static slice parameter.
     * ``value_type`` and ``N`` depend on the size of the bitset. This is the fastest
     * way to change a bitset bigger than a byte, but requires the slice to be the same
     * size (``N``) of the bitset.
     */
    void assign(value_slice const& values) { _data.assign(values); }

    /**
     * @brief
     * Assigns all bits to the same values as the bits in the dynamic slice parameter.
     * This is slower than ``assign``, but memory alignment safe when having a small
     * type slice which cannot safely be cast down to the ``value_type`` needed by
     * ``assign()``. If the slice's ``size()`` is bigger than the bitset's ``size() /
     * 8``, the slice will be trimmed and as such the last bytes will be reset.
     * @attention This is an endian-dependent operation, it will give different
     * resulting bitsets on big and little endian machines with the same input slice.
     * You must know that the source of bytes you're copying from is in the correct
     * format for the endianness of your target machine, otherwise your bitset will be
     * interpreted differently than you expect it to be.
     */
    void from_bytes(::estd::slice<uint8_t const> const& values) { _data.from_bytes(values); }

    /**
     * Returns the integral value of this bitset, if Size fits into value_type.
     */
    value_type value() const { return _data.value(); }

    /**
     * Returns the underlying data slice
     */
    value_slice to_slice() const { return _data.to_slice(); }

    /**
     * Returns the number of bits the bitset holds.
     */
    size_t size() const { return Size; }

    /**
     * Sets all bits to 1.
     */
    bitset& set()
    {
        _data.set();
        return *this;
    }

    /**
     * Sets the bit at pos to a given value.
     */
    bitset& set(size_t const pos, bool const value = true)
    {
        _data.set(pos, value);
        return *this;
    }

    /**
     * Resets all bits to 0.
     */
    bitset& reset()
    {
        _data.reset();
        return *this;
    }

    /**
     * Resets the bit at a given position to 0.
     */
    bitset& reset(size_t const pos)
    {
        _data.reset(pos);
        return *this;
    }

    /**
     * Returns whether the bit at position pos is set.
     */
    bool test(size_t const pos) const { return _data.test(pos); }

    /**
     * Returns whether all bits are set.
     */
    bool all() const { return _data.all(); }

    /**
     * Returns whether any bit is set.
     */
    bool any() const { return _data.any(); }

    /**
     * Returns whether no bit is set.
     */
    bool none() const { return !any(); }

    /**
     * Returns the value to the bit at position pos.
     */
    bool operator[](size_t const pos) const { return _data.test(pos); }

    /**
     * Returns a writable reference to the bit at position pos.
     */
    reference operator[](size_t const pos) { return _data.make_ref(pos); }

    /**
     * Inverts all bits.
     */
    bitset& flip()
    {
        _data.flip();
        return *this;
    }

    /**
     * Flips the bit at a given position.
     */
    bitset& flip(size_t const pos)
    {
        if (pos < Size)
        {
            reference r = _data.make_ref(pos);
            r           = ~r;
        }
        return *this;
    }

    size_t count() { return _data.count(); }

    bitset& operator&=(bitset const& rhs)
    {
        _data.operator&=(rhs._data);
        return *this;
    }

    bitset& operator|=(bitset const& rhs)
    {
        _data.operator|=(rhs._data);
        return *this;
    }

    bitset& operator^=(bitset const& rhs)
    {
        _data.operator^=(rhs._data);
        return *this;
    }

    bitset& operator>>=(size_t const pos)
    {
        _data.operator>>=(pos);
        return *this;
    }

    bitset operator>>(size_t const pos) const
    {
        auto tmp = *this;
        tmp >>= pos;
        return tmp;
    }

    bitset& operator<<=(size_t const pos)
    {
        _data.operator<<=(pos);
        return *this;
    }

    bitset operator<<(size_t const pos) const
    {
        auto tmp = *this;
        tmp <<= pos;
        return tmp;
    }

    bitset operator~() const
    {
        bitset b(*this);
        return b.flip();
    }

    bool operator==(bitset const& rhs) const { return _data.operator==(rhs._data); }

    bool operator!=(bitset const& rhs) const { return !(*this == rhs); }

private:
    data_type _data;
};

template<size_t Size>
bitset<Size> operator&(bitset<Size> const& lhs, bitset<Size> const& rhs)
{
    bitset<Size> result(lhs);
    result.operator&=(rhs);
    return result;
}

template<size_t Size>
bitset<Size> operator|(bitset<Size> const& lhs, bitset<Size> const& rhs)
{
    bitset<Size> result(lhs);
    result.operator|=(rhs);
    return result;
}

template<size_t Size>
bitset<Size> operator^(bitset<Size> const& lhs, bitset<Size> const& rhs)
{
    bitset<Size> result(lhs);
    result.operator^=(rhs);
    return result;
}

} // namespace estd

#endif /* GUARD_0FFBC651_ED5E_436C_BAA9_FCE7EAABE3BA */
