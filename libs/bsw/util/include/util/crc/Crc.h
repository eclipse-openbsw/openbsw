// Copyright 2024 Accenture.

#ifndef GUARD_90DB1B8F_E574_4B6E_9600_63330A15ECEE
#define GUARD_90DB1B8F_E574_4B6E_9600_63330A15ECEE

#include "util/crc/LookupTable.h"
#include "util/crc/Reflect.h"
#include "util/crc/Xor.h"

#include <estd/assert.h>

#include <cstddef>
#include <cstdint>

namespace util
{
namespace crc
{
template<
    class DigestType,
    bool ReflectInput   = false,
    bool ReflectOutput  = false,
    DigestType FinalXor = 0>
struct CrcCalculatorBase
{
    static DigestType update(
        DigestType const (&lookupTable)[256],
        uint8_t const* const data,
        size_t const length,
        DigestType const initValue = DigestType())
    {
        DigestType reg = initValue;
        estd_assert(data != nullptr || length == 0);
        for (size_t idx = 0U; idx < length; ++idx)
        {
            uint8_t const value            = Reflect<ReflectInput>::apply(data[idx]);
            uint8_t const lookupTableIndex = value ^ (reg >> ((sizeof(DigestType) * 8U) - 8U));
            reg = lookupTable[lookupTableIndex] ^ static_cast<DigestType>(reg << 8U);
        }
        return reg;
    }

    static DigestType digest(DigestType const value)
    {
        return Xor<DigestType, FinalXor>::apply(Reflect<ReflectOutput>::apply(value));
    }
};

template<
    class DigestType,
    DigestType Polynom,
    bool ReflectInput   = false,
    bool ReflectOutput  = false,
    DigestType FinalXor = 0>
struct CrcCalculator : public CrcCalculatorBase<DigestType, ReflectInput, ReflectOutput, FinalXor>
{
    using CrcCalculatorBase_t
        = CrcCalculatorBase<DigestType, ReflectInput, ReflectOutput, FinalXor>;

    static DigestType update(
        uint8_t const* const data, size_t const length, DigestType const initValue = DigestType())
    {
        return CrcCalculatorBase_t::update(
            LookupTable<DigestType, Polynom>::getTable(), data, length, initValue);
    }
};

template<
    class DigestType,
    DigestType Polynom,
    DigestType InitValue = 0,
    bool ReflectInput    = false,
    bool ReflectOutput   = false,
    DigestType FinalXor  = 0>
class CrcRegister
{
public:
    using CrcCalculator_t
        = CrcCalculator<DigestType, Polynom, ReflectInput, ReflectOutput, FinalXor>;

    inline CrcRegister() : _register(InitValue) {}

    inline void init(DigestType const value = InitValue) { _register = value; }

    /**
     *  Generic `update()` method using a `SliceType` embracing pointer and length.
     *
     *  `SliceType` needs to implement `.data()` and `.size()` returning a `uint8_t*` to data and
     *  a `size_t` respectively, e.g. `estd::slice<uint8_t const>`
     */
    template<class SliceType>
    inline DigestType update(SliceType const& data)
    {
        return update(data.data(), data.size());
    }

    inline DigestType update(uint8_t const* const data, size_t const length)
    {
        _register = CrcCalculator_t::update(data, length, _register);
        return _register;
    }

    inline DigestType digest() const { return CrcCalculator_t::digest(_register); }

private:
    DigestType _register;
};

} // namespace crc
} // namespace util

#endif /* GUARD_90DB1B8F_E574_4B6E_9600_63330A15ECEE */
