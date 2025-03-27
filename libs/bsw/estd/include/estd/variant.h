// Copyright 2024 Accenture.

/**
 * Contains estd::variant.
 * \file
 * \ingroup estl_containers
 */
#pragma once

#include "estd/algorithm.h"
#include "estd/assert.h"
#include "estd/constructor.h"
#include "estd/type_list.h"

#include <platform/estdint.h>

#include <cstring>
#include <new>
#include <type_traits>

namespace estd
{
namespace internal
{
template<typename R>
struct return_helper
{
    static R help()
    {
        estd_assert(false); // should be impossible to get here
        uint8_t const data[1]{};
        return *reinterpret_cast<R const*>(data);
    }
};

template<>
struct return_helper<void>
{
    static void help() {}
};

template<typename T>
struct variant_ops
{
    static bool compare(uint8_t const* const lhs, uint8_t const* const rhs)
    {
        return *reinterpret_cast<T const*>(lhs) == *reinterpret_cast<T const*>(rhs);
    }

    static void copy(uint8_t* const mem, uint8_t const* const rhs)
    {
        new (mem) T(*reinterpret_cast<T const*>(rhs));
    }

    static void destroy(uint8_t* const mem) { (*reinterpret_cast<T const*>(mem)).~T(); }

    template<bool>
    static void assign(uint8_t* const lhs, uint8_t const* const rhs)
    {
        *reinterpret_cast<T*>(lhs) = *reinterpret_cast<T const*>(rhs);
    }

    template<typename Visitor, typename R>
    static R call(uint8_t const* const mem, Visitor& visitor)
    {
        return visitor(*reinterpret_cast<T const*>(mem));
    }
};

template<>
struct variant_ops<void>
{
    static void copy(uint8_t* const, uint8_t const* const) {}

    static bool compare(uint8_t const* const, uint8_t const* const) { return false; }

    static void destroy(uint8_t* const) {}

    // clang-format off
    template<bool>       static void assign(uint8_t* const, uint8_t const* const) {}
    template<typename Visitor, typename R>
    static R call(uint8_t const* const, Visitor&) { return return_helper<R>::help(); }

    // clang-format on
};
} // namespace internal

template<typename TypeList, size_t ID = 0>
struct variant_T_do
{
    using T       = typename TypeList::type;
    using recurse = variant_T_do<typename TypeList::tail, ID + 1>;

    static void copy(size_t const t, uint8_t* const mem, uint8_t const* const rhs)
    {
        return (t == ID) ? internal::variant_ops<T>::copy(mem, rhs) : recurse::copy(t, mem, rhs);
    }

    static bool compare(size_t const t, uint8_t const* const lhs, uint8_t const* const rhs)
    {
        return (t == ID) ? internal::variant_ops<T>::compare(lhs, rhs)
                         : recurse::compare(t, lhs, rhs);
    }

    static void destroy(size_t const t, uint8_t* const mem)
    {
        return (t == ID) ? internal::variant_ops<T>::destroy(mem) : recurse::destroy(t, mem);
    }

    template<bool TriviallyCopyable>
    static void assign(
        size_t const t,
        uint8_t* const lhs,
        uint8_t const* const rhs,
        typename std::enable_if<!TriviallyCopyable>::type const* const = nullptr)
    {
        return (t == ID) ? internal::variant_ops<T>::template assign<TriviallyCopyable>(lhs, rhs)
                         : recurse::template assign<TriviallyCopyable>(t, lhs, rhs);
    }

    template<bool TriviallyCopyable>
    static void assign(
        size_t const,
        uint8_t* const lhs,
        uint8_t const* const rhs,
        typename std::enable_if<TriviallyCopyable>::type const* const = nullptr)
    {
        (void)::memcpy(lhs, rhs, max_size<TypeList>::value);
    }

    template<typename Visitor, typename R>
    static R call(size_t const t, uint8_t const* const mem, Visitor& visitor)
    {
        return (t == ID) ? internal::variant_ops<T>::template call<Visitor, R>(mem, visitor)
                         : recurse::template call<Visitor, R>(t, mem, visitor);
    }
};

// The next warning is a false positive because C++03 local types cannot be used inside function
// templates.
struct LT_visitor
{
    LT_visitor(uint8_t const* const mem) : mem(mem) {}

    template<typename T>
    bool operator()(T const& v) const
    {
        return (v < *reinterpret_cast<T const*>(mem));
    }

    uint8_t const* mem;
};

template<size_t ID>
struct variant_T_do<type_list_end, ID>
{
    static bool compare(size_t const, uint8_t const* const, uint8_t const* const) { return false; }

    template<bool>
    static void assign(size_t const, uint8_t* const, uint8_t const* const)
    {}

    static void copy(size_t const, uint8_t* const, uint8_t const* const) {}

    static void destroy(size_t const, uint8_t* const) {}

    template<typename F>
    static void construct(size_t const, uint8_t* const, F const)
    {}

    template<typename Visitor, typename R>
    static R call(size_t const, uint8_t const* const, Visitor&)
    {
        return internal::return_helper<R>::help();
    }
};

template<typename R, typename Visitor, typename Variant>
R visit(Visitor const& visitor, Variant const& v)
{
    return v.template visit<R, Visitor const>(visitor);
}

template<typename R, typename Visitor, typename Variant>
R visit(Visitor& visitor, Variant const& v)
{
    return v.template visit<R, Visitor>(visitor);
}

template<typename... Types>
class variant
{
    using VariantTypes = typename flat_type_list<Types...>::type;

    alignas(VariantTypes::max_align()) uint8_t _data[VariantTypes::max_size()];

    size_t _t;

    template<typename TypeList>
    inline void assign(
        variant const& rhs,
        typename std::enable_if<
            all_types<TypeList, std::is_trivially_copyable>::value>::type const* const
        = nullptr)
    {
        // TriviallyCopyable named requirement implies the type has
        // trivial non-default destructor; thus, we do not need to call the
        // destructor. Skip straight to assignment.
        _t = rhs._t;
        variant_T_do<VariantTypes>::template assign<true>(_t, _data, rhs._data);
    }

    template<typename TypeList>
    inline void assign(
        variant const& rhs,
        typename std::enable_if<
            (all_types<TypeList, std::is_trivially_copyable>::value == 0)>::type const* const
        = nullptr)
    {
        if (_t != rhs._t)
        {
            variant_T_do<VariantTypes>::destroy(_t, _data);
            _t = rhs._t;
            variant_T_do<VariantTypes>::copy(_t, _data, rhs._data);
        }
        else
        {
            variant_T_do<VariantTypes>::template assign<false>(_t, _data, rhs._data);
        }
    }

public:
    template<typename T>
    variant(T const& v) : _t(index_of<T>())
    {
        new (_data) T(v);
    }

    variant() : variant(typename VariantTypes::type()) {}

    variant(variant const& v) : _t(v._t) { variant_T_do<VariantTypes>::copy(_t, _data, v._data); }

    ~variant() { variant_T_do<VariantTypes>::destroy(_t, _data); }

    template<typename R, typename Visitor>
    R visit(Visitor& visitor) const
    {
        return variant_T_do<VariantTypes>::template call<Visitor, R>(_t, _data, visitor);
    }

    template<typename R, typename Visitor>
    R visit(Visitor const& visitor) const
    {
        return variant_T_do<VariantTypes>::template call<Visitor const, R>(_t, _data, visitor);
    }

    template<class T>
    variant& operator=(T const& rhs)
    {
        if (_t != index_of<T>())
        {
            variant_T_do<VariantTypes>::destroy(_t, &_data[0]);
            _t = index_of<T>();
            new (_data) T(rhs);
        }
        else
        {
            *reinterpret_cast<T*>(_data) = rhs;
        }
        return *this;
    }

    variant& operator=(variant const& rhs)
    {
        assign<VariantTypes>(rhs);
        return *this;
    }

    template<typename T>
    constructor<T> emplace()
    {
        variant_T_do<VariantTypes>::destroy(_t, _data);
        _t = index_of<T>();
        return constructor<T>(_data);
    }

    template<typename T>
    static size_t index_of()
    {
        static_assert(
            VariantTypes::template contains<T>(), "type has to be in variant's type list");
        return VariantTypes::template index_of<T>();
    }

    size_t index() const { return _t; }

    template<typename T>
    bool is() const
    {
        return _t == index_of<T>();
    }

    template<typename T>
    T& get()
    {
        static_assert(contains<VariantTypes, T>::value, "type has to be in variant's type list");
        estd_assert(is<T>());
        return *reinterpret_cast<T*>(_data);
    }

    template<typename T>
    T const& get() const
    {
        static_assert(contains<VariantTypes, T>::value, "type has to be in variant's type list");
        estd_assert(is<T>());
        return *reinterpret_cast<T const*>(_data);
    }

    template<typename T>
    bool operator!=(T const& rhs) const
    {
        // no static assert required here
        return !(*this == rhs);
    }

    bool operator==(variant const& rhs) const
    {
        return (_t == rhs._t) && variant_T_do<VariantTypes>::compare(_t, _data, rhs._data);
    }

    bool operator<(variant const& rhs) const
    {
        if (_t < rhs._t)
        {
            return true;
        }
        if (_t > rhs._t)
        {
            return false;
        }
        return visit<bool>(LT_visitor(rhs._data));
    }
};

template<typename T, typename V>
constexpr bool holds_alternative(V const& v)
{
    return v.template is<T>();
}

template<typename T, typename V>
constexpr T const& get(V const& v)
{
    return v.template get<T>();
}

template<typename T, typename Variant, typename F>
T value_or_else(Variant& v, F const& f)
{
    if (v.template is<T>())
    {
        return v.template get<T>();
    }
    return f();
}

} // namespace estd
