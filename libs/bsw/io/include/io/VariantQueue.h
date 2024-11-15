// Copyright 2024 Accenture.

#ifndef GUARD_C512CEED_E256_4221_B8CB_BB6E8451A7C7
#define GUARD_C512CEED_E256_4221_B8CB_BB6E8451A7C7

#include <etl/byte_stream.h>
#include <etl/largest.h>
#include <etl/memory.h>
#include <etl/parameter_pack.h>
#include <etl/span.h>
#include <etl/type_traits.h>
#include <etl/variant.h>
#include <io/MemoryQueue.h>

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
template<typename T, typename... TRest>
struct max_element_size
{
    static constexpr size_t value = ::etl::max<size_t>(
        sizeof(typename T::type) + T::max_payload_size, max_element_size<TRest...>::value);
};

template<typename T>
struct max_element_size<T>
{
    static constexpr size_t value = sizeof(typename T::type) + T::max_payload_size;
};
} // namespace internal

// type list
template<typename... ElementTypes>
struct make_variant_queue
{
    using type_list = typename ::etl::parameter_pack<typename ElementTypes::type...>;

    static constexpr size_t queue_max_element_type
        = internal::max_element_size<ElementTypes...>::value;

    template<typename T>
    using contains = ::etl::is_one_of<T, typename ElementTypes::type...>;

    enum
    {
        max_align = ::etl::largest<typename ElementTypes::type...>::alignment,
        size      = type_list::size
    };
};

template<typename QueueTypeList, size_t CAPACITY>
using VariantQueue
    = ::io::MemoryQueue<CAPACITY, 1 + QueueTypeList::queue_max_element_type, uint16_t>;

template<typename TypeList>
struct variant_q
{
    static_assert(
        TypeList::max_align == 1,
        "structs are directly serialized into the queue, alignment other than 1-byte cannot be "
        "guaranteed");

    using types = TypeList;

private:
    template<size_t ID = 0>
    struct variant_do
    {
        using T       = typename TypeList::type_list::type_from_index<ID>::type;
        using recurse = variant_do<ID + 1 == TypeList::size ? ID : ID + 1>;

        template<typename Visitor, typename R>
        static void call(size_t const t, ::etl::span<uint8_t const> const mem, Visitor& visitor)
        {
            if (t < TypeList::size)
            {
                if (t == ID)
                {
                    visitor(*reinterpret_cast<T const*>(mem.data()));
                }
                else
                {
                    recurse::template call<Visitor, R>(t, mem, visitor);
                }
            }
        }

        template<typename Visitor, typename R>
        static void
        call_with_payload(size_t const t, ::etl::span<uint8_t const> const mem, Visitor& visitor)
        {
            if (t < TypeList::size)
            {
                if (t == ID)
                {
                    visitor(*reinterpret_cast<T const*>(mem.data()), mem.subspan(sizeof(T)));
                }
                else
                {
                    recurse::template call_with_payload<Visitor, R>(t, mem, visitor);
                }
            }
        }
    };

    template<typename T>
    static void write_header(T const& t, ::etl::span<uint8_t>& buffer)
    {
        static_assert(
            TypeList::template contains<T>::value, "type must be a part of the variant type list");

        buffer[0] = static_cast<uint8_t>(TypeList::type_list::template index_of_type<T>::value);
        *(reinterpret_cast<T*>(&buffer[1])) = t;
        buffer                              = buffer.subspan(1 + sizeof(t));
    }

public:
    template<typename Visitor>
    static void read(Visitor& visitor, ::etl::span<uint8_t const> const data)
    {
        assert(data.size() != 0);

        return variant_do<>::template call<Visitor, void>(data[0], data.subspan(1), visitor);
    }

    template<typename Visitor>
    static void read_with_payload(Visitor& visitor, ::etl::span<uint8_t const> const data)
    {
        assert(data.size() != 0);
        return variant_do<>::template call_with_payload<Visitor, void>(
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
        (void)::etl::mem_copy(payload.begin(), payload.size(), buffer.begin());

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
            TypeList::template contains<T>::value, "type must be a part of the variant type list");

        auto const buffer = w.allocate(sizeof(T) + 1);
        if (buffer.size() == 0)
        {
            return nullptr;
        }
        buffer[0] = static_cast<uint8_t>(TypeList::type_list::template index_of_type<T>::value);
        return reinterpret_cast<T*>(&buffer[1]);
    }
};

} // namespace io

#endif // GUARD_C512CEED_E256_4221_B8CB_BB6E8451A7C7
