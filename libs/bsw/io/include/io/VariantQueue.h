// Copyright 2024 Accenture.

#pragma once

#include <io/MemoryQueue.h>

#include <estd/memory.h>
#include <estd/type_list.h>
#include <estd/type_traits.h>
#include <estd/variant.h>

#include <cassert>
#include <type_traits>

/**
 * The idea of a "variant queue" here is to encode multiple frame types, with attached
 * dynamically-sized payloads and transmit them via MemoryQueue.
 *
 * Encoding looks like this:
 *            uint8                 | N bytes      | M bytes
 *  type id (derived from TypeList) | frame/header | payload
 *
 * In this file you can find few helper structures and functions for operating on a queue with this
 * encoding.
 *
 * Note - structs used as headers need to be PODs. Unless unaligned read/write is acceptable on the
 * target platform, they should also have 1-byte alignment requirement.
 *
 * See the io module documentation for more info and examples.
 */

namespace io
{
template<typename T, size_t MAX_PAYLOAD_SIZE = 0>
struct VariantQueueType
{
    static constexpr size_t max_payload_size = MAX_PAYLOAD_SIZE;
    using type                               = T;
};

namespace internal
{
template<typename T>
class element_size
{
    template<typename C>
    static constexpr size_t size(decltype(&C::max_payload_size))
    {
        return sizeof(typename C::type) + C::max_payload_size;
    }

    template<typename C>
    static constexpr size_t size(...)
    {
        return sizeof(typename C::type);
    }

public:
    static constexpr size_t value = size<T>(nullptr);
};

template<typename TL>
struct max_element_size
{
    static constexpr size_t value = ::estd::max<size_t>(
        element_size<typename TL::type>::value, max_element_size<typename TL::tail>::value);
};

template<>
struct max_element_size<::estd::type_list_end>
{
    static constexpr size_t value = 0;
};
} // namespace internal

template<typename... ElementTypes>
struct make_variant_queue
{
    using type_list = typename ::estd::make_type_list<typename ElementTypes::type...>::type;

    static constexpr size_t queue_max_element_type
        = internal::max_element_size<typename ::estd::make_type_list<ElementTypes...>::type>::value;
};

template<typename QueueTypeList, size_t CAPACITY>
using VariantQueue
    = ::io::MemoryQueue<CAPACITY, 1 + QueueTypeList::queue_max_element_type, uint16_t>;

template<typename TypeList>
struct variant_q
{
    static_assert(
        ::estd::max_align<TypeList>::value == 1,
        "structs are directly serialized into the queue, alignment other than 1-byte cannot be "
        "guaranteed");

    using types = TypeList;

private:
    template<typename TL, size_t ID = 0>
    struct variant_do
    {
        using T       = typename TL::type;
        using recurse = variant_do<typename TL::tail, ID + 1>;

        template<typename Visitor, typename R>
        static void call(size_t const t, ::estd::slice<uint8_t const> const mem, Visitor& visitor)
        {
            if (t == ID)
            {
                visitor(*reinterpret_cast<T const*>(mem.data()), mem.offset(sizeof(T)));
            }
            else
            {
                recurse::template call<Visitor, R>(t, mem, visitor);
            }
        }
    };

    template<size_t ID>
    struct variant_do<::estd::type_list_end, ID>
    {
        template<typename Visitor, typename R>
        static void call(size_t const, ::estd::slice<uint8_t const> const, Visitor const&)
        {}
    };

    template<typename T>
    static void write_header(T const& t, ::estd::slice<uint8_t>& buffer)
    {
        static_assert(
            ::estd::contains<TypeList, T>::value, "type must be a part of the variant type list");
        static_assert(std::is_trivial<T>::value, "type must be trivial");

        ::estd::memory::take<uint8_t>(buffer)
            = static_cast<uint8_t>(::estd::index_of<TypeList, T>::value);
        ::estd::memory::take<T>(buffer) = t;
    }

public:
    template<typename Visitor>
    static void read(Visitor& visitor, ::estd::slice<uint8_t const> const data)
    {
        assert(data.size() != 0);
        return ::estd::variant_T_do<TypeList>::template call<Visitor, void>(
            data[0], data.offset(1).data(), visitor);
    }

    template<typename Visitor>
    static void read_with_payload(Visitor& visitor, ::estd::slice<uint8_t const> const data)
    {
        assert(data.size() != 0);
        return variant_do<TypeList>::template call<Visitor, void>(data[0], data.offset(1), visitor);
    }

    template<typename T, typename Writer>
    static bool write(Writer& w, T const& t)
    {
        auto buffer = w.allocate(sizeof(T) + 1);
        if (buffer.size() == 0)
        {
            return false;
        }
        write_header(t, buffer);
        w.commit();
        return true;
    }

    template<typename T, typename Writer, typename F>
    static bool write(Writer& w, T const& t, size_t const payloadSize, F const fillPayload)
    {
        auto buffer = w.allocate(sizeof(T) + 1 + payloadSize);
        if (buffer.size() == 0)
        {
            return false;
        }

        write_header(t, buffer);
        fillPayload(buffer);
        w.commit();
        return true;
    }

    template<typename T, typename Writer>
    static bool write(Writer& w, T const& t, ::estd::slice<uint8_t const> const payload)
    {
        auto buffer = w.allocate(sizeof(T) + 1 + payload.size());
        if (buffer.size() == 0)
        {
            return false;
        }

        write_header(t, buffer);
        (void)::estd::memory::copy(buffer, payload);

        w.commit();
        return true;
    }

    template<typename T, typename Writer>
    static ::estd::slice<uint8_t> alloc_payload(Writer& w, T const& t, size_t const payloadSize)
    {
        auto buffer = w.allocate(sizeof(T) + 1 + payloadSize);
        if (buffer.size() != 0)
        {
            write_header(t, buffer);
        }
        return buffer;
    }

    template<typename T, typename Writer>
    static T* alloc_header(Writer& w)
    {
        static_assert(
            ::estd::contains<TypeList, T>::value, "type must be a part of the variant type list");
        static_assert(std::is_trivial<T>::value, "type must be trivial");

        auto const buffer = w.allocate(sizeof(T) + 1);
        if (buffer.size() == 0)
        {
            return nullptr;
        }
        buffer[0] = static_cast<uint8_t>(::estd::index_of<TypeList, T>::value);
        return reinterpret_cast<T*>(&buffer[1]);
    }
};

} // namespace io

