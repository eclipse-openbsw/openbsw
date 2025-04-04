// Copyright 2024 Accenture.

/**
 * \file Mask.h
 */
#pragma once

#include <platform/estdint.h>

// This class is in the global scope for backwards compatibility
/**
 * Generic bitmask helper class
 *
 */
template<typename T, typename V = uint8_t>
class Mask
{
public:
    using mask_type = V;
    static_assert((sizeof(V) * 8) >= T::MAX_INDEX, "");

    Mask(V mask = V());

    Mask(Mask const& mask) : fMask(mask.fMask) {}

    Mask& operator=(Mask const& mask)
    {
        fMask = mask.fMask;
        return *this;
    }

    Mask const operator&(Mask const& mask) const
    {
        V const result = static_cast<V>(fMask & mask.fMask);
        return Mask(result);
    }

    Mask const operator|(Mask const& mask) const
    {
        V const result = static_cast<V>(fMask | mask.fMask);
        return Mask(result);
    }

    Mask& operator|=(Mask const& mask)
    {
        fMask |= mask.fMask;
        return *this;
    }

    bool operator==(Mask const& mask) const { return fMask == mask.getMask(); }

    bool operator!=(Mask const& mask) const { return fMask != mask.getMask(); }

    V getMask() const { return fMask; }

    void setMask(V mask);

    void clear() { fMask = 0U; }

    bool isSet(V b) const;

    bool match(T const& t) const
    {
        V const v = static_cast<V>(1U) << t.toIndex();
        return (fMask & v) > 0U;
    }

    bool overlaps(Mask const& mask) const { return ((fMask & mask.getMask()) != 0U); }

    Mask& add(T const& t) { return (*this << t); }

    Mask& remove(T const& t) { return (*this >> t); }

    Mask& operator<<(T const& t)
    {
        V const v = static_cast<V>(1U) << t.toIndex();
        fMask     = static_cast<V>(fMask | v);
        return *this;
    }

    Mask& operator>>(T const& t)
    {
        V const v = static_cast<V>(1U) << t.toIndex();
        fMask     = (fMask & static_cast<V>(~v));
        return *this;
    }

    static V getOpenMask() { return ~static_cast<V>(0); }

    static Mask& getInstance()
    {
        static Mask theMaskInstance;
        theMaskInstance.clear();
        return theMaskInstance;
    }

private:
    V fMask;
};

/*
 * inline
 */

template<typename T, typename V>
inline Mask<T, V>::Mask(V const mask) : fMask(mask)
{}

template<typename T, typename V>
inline void Mask<T, V>::setMask(V const mask)
{
    fMask = mask;
}

template<typename T, typename V>
inline bool Mask<T, V>::isSet(V const b) const
{
    V const v = static_cast<V>(1U) << b;
    return (fMask & v) > 0U;
}
