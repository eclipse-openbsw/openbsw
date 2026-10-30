/********************************************************************************
 * Copyright (c) 2026 KPIT Technologies Ltd.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "someip/EtlArray.h"
#include "someip/EtlVector.h"
#include "someip/ISomeIpSerializable.h"
#include "someip/SomeIpParser.h"
#include "someip/SomeIpSerializer.h"

#include <etl/span.h>
#include <etl/type_traits.h>

#include <cstddef>
#include <cstdint>

namespace middleware::bindings::someip
{

/**
 * SOME/IP wire codec for a middleware payload, keyed on Tag, where Tag::Value is
 * the C++ payload type. It is keyed on a Tag rather than on the type because one
 * type can need different encodings on different members: the same byte vector
 * backs the 8-, 16- and 32-bit length-prefix variants.
 *
 * Must be specialized for every payload type the service uses, except scalars,
 * which are streamed natively and need no Tag. Field order and width must match
 * the type's definition in the generated header; nothing checks that, so a
 * mismatch is a silent wire-format bug.
 */
template<typename Tag>
struct SomeIpCodec
{
    using Value = typename Tag::Value;

    static void serialize(Value const& value, ::someip::SomeIpSerializer& out);
    static void parse(Value& value, ::someip::SomeIpParser& in);
    static uint32_t size(Value const& value);
};

namespace internal
{

/** True for the primitives the serializer and parser stream natively. */
template<typename T>
struct IsNativelyStreamable : ::etl::bool_constant<false>
{};

// clang-format off
template<> struct IsNativelyStreamable<bool>     : ::etl::bool_constant<true> {};
template<> struct IsNativelyStreamable<char>     : ::etl::bool_constant<true> {};
template<> struct IsNativelyStreamable<uint8_t>  : ::etl::bool_constant<true> {};
template<> struct IsNativelyStreamable<int8_t>   : ::etl::bool_constant<true> {};
template<> struct IsNativelyStreamable<uint16_t> : ::etl::bool_constant<true> {};
template<> struct IsNativelyStreamable<int16_t>  : ::etl::bool_constant<true> {};
template<> struct IsNativelyStreamable<uint32_t> : ::etl::bool_constant<true> {};
template<> struct IsNativelyStreamable<int32_t>  : ::etl::bool_constant<true> {};
template<> struct IsNativelyStreamable<uint64_t> : ::etl::bool_constant<true> {};
template<> struct IsNativelyStreamable<int64_t>  : ::etl::bool_constant<true> {};
template<> struct IsNativelyStreamable<float>    : ::etl::bool_constant<true> {};
template<> struct IsNativelyStreamable<double>   : ::etl::bool_constant<true> {};

// clang-format on

} // namespace internal

namespace internal
{

/** Identity tag, so a plain scalar needs no Tag or specialization of its own. */
template<typename T>
struct ScalarTag
{
    using Value = T;
};

} // namespace internal

/**
 * Adapts a middleware payload value to ISomeIpSerializable.
 *
 * \tparam T   Payload type.
 * \tparam Tag Selects the wire codec. Defaults to the scalar identity tag; pass
 *             an explicit Tag for any non-scalar type, or for a type needing a
 *             non-default encoding such as a particular length-prefix width.
 */
template<typename T, typename Tag = internal::ScalarTag<T>>
class SomeIpSerializableValue final : public ::someip::ISomeIpSerializable
{
public:
    static_assert(
        ::etl::is_same<typename Tag::Value, T>::value, "Tag::Value must match the payload type T");

    /**
     * Stream T directly only when it is a primitive and no explicit Tag was
     * given. Keying on the type alone would ignore a Tag supplied for a scalar,
     * which is the only way a scalar can carry extra wire rules.
     */
    static constexpr bool USES_CODEC
        = !(internal::IsNativelyStreamable<T>::value
            && ::etl::is_same<Tag, internal::ScalarTag<T>>::value);

    SomeIpSerializableValue() = default;

    explicit SomeIpSerializableValue(T const& value) : _value(value) {}

    T const& get() const { return _value; }

    T& get() { return _value; }

    void set(T const& value) { _value = value; }

    /**
     * Serializes \p source in place of the held value until called with nullptr,
     * so a value that already lives elsewhere goes on the wire without a copy.
     * Parsing still fills the held value.
     */
    void serializeFrom(T const* const source) { _source = source; }

    void serializeToArray(::someip::SomeIpSerializer& serializer) const override
    {
        if constexpr (!USES_CODEC)
        {
            serializer << outgoing();
        }
        else
        {
            SomeIpCodec<Tag>::serialize(outgoing(), serializer);
        }
    }

    /**
     * A payload holds exactly one serialized value, so bytes left over after it
     * mean the sender and this type disagree, most often a length prefix shorter
     * than the data behind it. Failing the parser here is what turns that into
     * E_MALFORMED_MESSAGE, which no per-type codec can do: each one reads what it
     * was told to read and stops.
     */
    void parseFromArray(::someip::SomeIpParser& parser) override
    {
        if constexpr (!USES_CODEC)
        {
            parser >> _value;
        }
        else
        {
            SomeIpCodec<Tag>::parse(_value, parser);
        }

        if (parser.isGood() && (parser.bytesAvailable() != 0U))
        {
            parser.setFailure();
        }
    }

    uint32_t getSize() const override
    {
        if constexpr (!USES_CODEC)
        {
            return static_cast<uint32_t>(sizeof(T));
        }
        else
        {
            return SomeIpCodec<Tag>::size(outgoing());
        }
    }

private:
    T const& outgoing() const { return (_source != nullptr) ? *_source : _value; }

    T _value{};
    T const* _source{nullptr};
};

/**
 * One encoding per kind of datatype, parameterized by what the model states
 * about it. A service's codec header specializes SomeIpCodec<Tag> by deriving
 * from one of these, so each encoding is written, and reviewed, once.
 */
namespace codec
{

/** Exactly N elements, no length prefix. */
template<typename T, typename Element, size_t N>
struct FixedArray
{
    static_assert(
        internal::IsNativelyStreamable<Element>::value,
        "Element must be a primitive, so it takes sizeof(Element) bytes on the wire");

    static void serialize(T const& value, ::someip::SomeIpSerializer& out)
    {
        ::someip::ArraySerializer<Element, N>::serialize(value, out, /*isBigEndian=*/true);
    }

    static void parse(T& value, ::someip::SomeIpParser& in)
    {
        ::someip::ArrayParser<Element, N>::parse(value, in, /*isBigEndian=*/true);
    }

    static uint32_t size(T const&) { return static_cast<uint32_t>(N * sizeof(Element)); }
};

/** A length prefix, then the elements. Fewer than MinLength is malformed. */
template<typename T, typename Element, typename Prefix, size_t MinLength>
struct DynamicArray
{
    static_assert(
        internal::IsNativelyStreamable<Element>::value,
        "Element must be a primitive, so it takes sizeof(Element) bytes on the wire");

    static void serialize(T const& value, ::someip::SomeIpSerializer& out)
    {
        ::someip::DynamicArraySerializer<Element, Prefix>::serialize(
            value, out, MinLength, /*isBigEndian=*/true);
    }

    static void parse(T& value, ::someip::SomeIpParser& in)
    {
        ::someip::DynamicArrayParser<Element, Prefix>::parse(
            value, in, MinLength, /*isBigEndian=*/true);
    }

    static uint32_t size(T const& value)
    {
        return static_cast<uint32_t>(sizeof(Prefix) + (value.size() * sizeof(Element)));
    }
};

/** An outer count, then each inner array with its own length prefix. */
template<
    typename T,
    typename Element,
    typename OuterPrefix,
    typename InnerPrefix,
    size_t InnerMinLength>
struct DynamicArray2d
{
    static_assert(
        internal::IsNativelyStreamable<Element>::value,
        "Element must be a primitive, so it takes sizeof(Element) bytes on the wire");

    static void serialize(T const& value, ::someip::SomeIpSerializer& out)
    {
        out.bigEndian();
        out << static_cast<OuterPrefix>(value.size());
        for (auto const& inner : value)
        {
            ::someip::DynamicArraySerializer<Element, InnerPrefix>::serialize(
                inner, out, InnerMinLength, /*isBigEndian=*/true);
        }
    }

    static void parse(T& value, ::someip::SomeIpParser& in)
    {
        value.clear();

        in.bigEndian();
        OuterPrefix outerCount = 0U;
        in >> outerCount;
        if (!in.isGood())
        {
            return;
        }

        for (OuterPrefix i = 0U; i < outerCount; ++i)
        {
            if (value.full())
            {
                value.clear();
                in.setFailure();
                return;
            }

            // Each row is parsed where it will live, not into a copy of it.
            ::someip::DynamicArrayParser<Element, InnerPrefix>::parse(
                value.emplace_back(), in, InnerMinLength, /*isBigEndian=*/true);
            if (!in.isGood())
            {
                value.clear();
                return;
            }
        }
    }

    static uint32_t size(T const& value)
    {
        uint32_t total = sizeof(OuterPrefix);
        for (auto const& inner : value)
        {
            total += static_cast<uint32_t>(sizeof(InnerPrefix) + (inner.size() * sizeof(Element)));
        }
        return total;
    }
};

/** The character encodings a string can declare. */
enum class Encoding : uint8_t
{
    Utf8,
    Utf16Be,
    Utf16Le
};

/** The byte order mark a string of each encoding starts with. */
template<Encoding E>
struct Bom;

template<>
struct Bom<Encoding::Utf8>
{
    static constexpr uint8_t BYTES[] = {0xEFU, 0xBBU, 0xBFU};
};

template<>
struct Bom<Encoding::Utf16Be>
{
    static constexpr uint8_t BYTES[] = {0xFEU, 0xFFU};
};

template<>
struct Bom<Encoding::Utf16Le>
{
    static constexpr uint8_t BYTES[] = {0xFFU, 0xFEU};
};

/** A string carrying a different mark is not of the encoding it declares. */
template<Encoding E, typename Bytes>
bool hasBom(Bytes const& value)
{
    constexpr size_t length = sizeof(Bom<E>::BYTES);
    if (value.size() < length)
    {
        return false;
    }
    for (size_t i = 0U; i < length; ++i)
    {
        if (value[i] != Bom<E>::BYTES[i])
        {
            return false;
        }
    }
    return true;
}

/** Exactly Length bytes, BOM and terminator included, whatever the encoding. */
template<typename T, size_t Length, Encoding E>
struct FixedString
{
    static void serialize(T const& value, ::someip::SomeIpSerializer& out)
    {
        ::someip::ArraySerializer<uint8_t, Length>::serialize(value, out, /*isBigEndian=*/true);
    }

    static void parse(T& value, ::someip::SomeIpParser& in)
    {
        ::someip::ArrayParser<uint8_t, Length>::parse(value, in, /*isBigEndian=*/true);
        if (in.isGood() && !hasBom<E>(value))
        {
            in.setFailure();
        }
    }

    static uint32_t size(T const&) { return static_cast<uint32_t>(Length); }
};

/** A length prefix, then the string with its BOM and terminator. */
template<typename T, typename Prefix, Encoding E>
struct DynamicString
{
    static void serialize(T const& value, ::someip::SomeIpSerializer& out)
    {
        ::someip::DynamicArraySerializer<uint8_t, Prefix>::serialize(
            value, out, /*minLength=*/0U, /*isBigEndian=*/true);
    }

    static void parse(T& value, ::someip::SomeIpParser& in)
    {
        ::someip::DynamicArrayParser<uint8_t, Prefix>::parse(
            value, in, /*minLength=*/0U, /*isBigEndian=*/true);
        if (in.isGood() && !hasBom<E>(value))
        {
            in.setFailure();
        }
    }

    static uint32_t size(T const& value)
    {
        return static_cast<uint32_t>(sizeof(Prefix) + value.size());
    }
};

/**
 * A scalar the model bounds by its enumerators: above Max is malformed.
 * setFailure() is what turns that into E_MALFORMED_MESSAGE, since the
 * provider's signature is the plain scalar and cannot refuse a value.
 */
template<typename T, T Max>
struct BoundedScalar
{
    static void serialize(T const& value, ::someip::SomeIpSerializer& out) { out << value; }

    static void parse(T& value, ::someip::SomeIpParser& in)
    {
        in >> value;
        if (in.isGood() && (value > Max))
        {
            in.setFailure();
        }
    }

    static uint32_t size(T const&) { return static_cast<uint32_t>(sizeof(T)); }
};

/**
 * A union carried as the bytes it arrived as: length, type tag, then the
 * member. The middleware moves the buffer; only the member table is checked.
 *
 * \tparam Members supplies isMember(type, length), true when type is declared
 *                 and length is its width, and correct(length), the declared
 *                 member of that width or 0 when there is none.
 */
template<typename T, typename Members>
struct Union
{
    /** Width of the type tag that follows the length field. */
    static constexpr uint32_t TAG_SIZE = 4U;

    static void serialize(T const& value, ::someip::SomeIpSerializer& out)
    {
        out << ::etl::span<uint8_t const>(value.data(), value.size());
    }

    /**
     * A type tag outside the member table is corrected to the member whose
     * width the payload actually has, rather than refused: the length says
     * unambiguously which member was meant.
     */
    static void parse(T& value, ::someip::SomeIpParser& in)
    {
        in.bigEndian();

        uint32_t length = 0U;
        in >> length;
        if (!in.isGood() || (length < TAG_SIZE))
        {
            in.setFailure();
            return;
        }

        uint32_t type = 0U;
        in >> type;
        if (!in.isGood())
        {
            return;
        }

        uint32_t const memberLength = length - TAG_SIZE;
        if (!Members::isMember(type, memberLength))
        {
            type = Members::correct(memberLength);
            if (type == 0U)
            {
                in.setFailure();
                return;
            }
        }

        value.clear();
        if (in.bytesAvailable() < memberLength)
        {
            in.setFailure();
            return;
        }
        append32(value, length);
        append32(value, type);
        auto const member = in.getAvailableBuffer().first(memberLength);
        value.insert(value.end(), member.begin(), member.end());
        in.skip(memberLength);
    }

    static uint32_t size(T const& value) { return static_cast<uint32_t>(value.size()); }

private:
    static void append32(T& out, uint32_t const v)
    {
        out.push_back(static_cast<uint8_t>((v >> 24U) & 0xFFU));
        out.push_back(static_cast<uint8_t>((v >> 16U) & 0xFFU));
        out.push_back(static_cast<uint8_t>((v >> 8U) & 0xFFU));
        out.push_back(static_cast<uint8_t>(v & 0xFFU));
    }
};

} // namespace codec

} // namespace middleware::bindings::someip
