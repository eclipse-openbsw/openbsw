// Copyright 2024 Accenture.

#pragma once

#include <etl/error_handler.h>
#include <etl/memory.h>
#include <etl/span.h>
#include <etl/type_list.h>
#include <etl/type_traits.h>
#include <etl/variant.h>
#include <io/MemoryQueue.h>

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
    static constexpr size_t value = ::etl::max<size_t>(
        element_size<typename TL::head>::value, max_element_size<typename TL::tail>::value);
};

template<>
struct max_element_size<::etl::type_list<>>
{
    static constexpr size_t value = 0;
};

template<typename R>
struct return_helper
{
    static R help() {}
};

template<typename T>
struct variant_ops
{
    template<typename Visitor, typename R>
    static R call(uint8_t const* const mem, Visitor& visitor)
    {
        return visitor(*reinterpret_cast<T const*>(mem));
    }
};

template<>
struct variant_ops<void>
{
    template<typename Visitor, typename R>
    static R call(uint8_t const* const, Visitor&)
    {
        return return_helper<R>::help();
    }
};
} // namespace internal

template<typename... ElementTypes>
struct make_variant_queue
{
    using type_list = typename ::etl::type_list<typename ElementTypes::type...>;

    static constexpr size_t queue_max_element_type
        = internal::max_element_size<typename ::etl::type_list<ElementTypes...>>::value;
};

template<typename QueueTypeList, size_t CAPACITY>
using VariantQueue
    = ::io::MemoryQueue<CAPACITY, 1 + QueueTypeList::queue_max_element_type, uint16_t>;

template<typename TypeList, size_t ID = 0>
struct variant_T_do
{
    using T       = typename TypeList::head;
    using recurse = variant_T_do<typename TypeList::tail, ID + 1>;

    template<typename Visitor, typename R>
    static R call(size_t const t, uint8_t const* const mem, Visitor& visitor)
    {
        return (t == ID) ? internal::variant_ops<T>::template call<Visitor, R>(mem, visitor)
                         : recurse::template call<Visitor, R>(t, mem, visitor);
    }
};

template<size_t ID>
struct variant_T_do<etl::type_list<>, ID>
{
    template<typename Visitor, typename R>
    static R call(size_t const, uint8_t const* const, Visitor&)
    {
        return internal::return_helper<R>::help();
    }
};

template<typename TypeList>
struct variant_q
{
    static_assert(
        ::etl::type_list_max_alignment<TypeList>::value == 1,
        "structs are directly serialized into the queue, alignment other than 1-byte cannot be "
        "guaranteed");

    using types = TypeList;

private:
    template<typename TL, size_t ID = 0>
    struct variant_do
    {
        using T       = typename TL::head;
        using recurse = variant_do<typename TL::tail, ID + 1>;

        template<typename Visitor, typename R>
        static void call(size_t const t, ::etl::span<uint8_t const> const mem, Visitor& visitor)
        {
            if (t == ID)
            {
                visitor(*reinterpret_cast<T const*>(mem.data()), mem.subspan(sizeof(T)));
            }
            else
            {
                recurse::template call<Visitor, R>(t, mem, visitor);
            }
        }
    };

    template<size_t ID>
    struct variant_do<::etl::type_list<>, ID>
    {
        template<typename Visitor, typename R>
        static void call(size_t const, ::etl::span<uint8_t const> const, Visitor const&)
        {}
    };

    template<typename T>
    static void write_header(T const& t, ::etl::span<uint8_t>& buffer)
    {
        static_assert(
            ::etl::type_list_contains<TypeList, T>::value,
            "type must be a part of the variant type list");

        buffer[0] = static_cast<uint8_t>(::etl::type_list_index_of_type<TypeList, T>::value);
        buffer.advance(1);
        buffer.reinterpret_as<T>()[0] = t;
        buffer.advance(sizeof(T));
    }

public:
    template<typename Visitor>
    static void read(Visitor& visitor, ::etl::span<uint8_t const> const data)
    {
        ETL_ASSERT(data.size() != 0, ETL_ERROR_GENERIC("Buffer empty"));
        return variant_T_do<TypeList>::template call<Visitor, void>(
            data[0], data.subspan(1).data(), visitor);
    }

    template<typename Visitor>
    static void read_with_payload(Visitor& visitor, ::etl::span<uint8_t const> const data)
    {
        ETL_ASSERT(data.size() != 0, ETL_ERROR_GENERIC("Buffer empty"));
        return variant_do<TypeList>::template call<Visitor, void>(
            data[0], data.subspan(1), visitor);
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
    static bool write(Writer& w, T const& t, ::etl::span<uint8_t const> const payload)
    {
        auto buffer = w.allocate(sizeof(T) + 1 + payload.size());
        if (buffer.size() == 0)
        {
            return false;
        }

        write_header(t, buffer);
        (void)::etl::copy(payload, buffer);

        w.commit();
        return true;
    }

    template<typename T, typename Writer>
    static ::etl::span<uint8_t> alloc_payload(Writer& w, T const& t, size_t const payloadSize)
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
            ::etl::type_list_contains<TypeList, T>::value,
            "type must be a part of the variant type list");

        auto const buffer = w.allocate(sizeof(T) + 1);
        if (buffer.size() == 0)
        {
            return nullptr;
        }
        buffer[0] = static_cast<uint8_t>(::etl::type_list_index_of_type<TypeList, T>::value);
        return reinterpret_cast<T*>(&buffer[1]);
    }
};

} // namespace io
