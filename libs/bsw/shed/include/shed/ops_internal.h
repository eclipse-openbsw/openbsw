/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "shed/id.h"
#include "shed/move_op.h"
#include "shed/multi_list.h"
#include "shed/table_internal.h"

#include <etl/delegate.h>
#include <etl/type_list.h>

#include <cstddef>
#include <new>
#include <type_traits>

namespace shed
{
namespace internal
{
template<typename StateData, typename T, typename First, typename... _Types>
struct find_container
{
    using type = typename find_container<StateData, T, _Types...>::type;
};

template<typename StateData, typename T, typename... _Types>
struct find_container<StateData, T, column<T>, _Types...>
{
    using type = column<T>;
};

template<typename StateData, typename T, typename... _Types>
struct find_container<StateData, T, column<T*>, _Types...>
{
    using type = column<T*>;
};

template<typename StateData, typename T, typename... _Types>
struct find_container<StateData, T const, column<T*>, _Types...>
{
    using type = column<T*>;
};

template<typename StateData, typename T, typename... _Types>
struct find_container<StateData, T**, column<T*>, _Types...>
{
    using type = column<T*>;
};

template<typename StateData, typename T, typename... _Types>
struct find_container<StateData, T const, column<T>, _Types...>
{
    using type = column<T>;
};

template<typename StateData, typename T, typename... _Types>
struct find_container<StateData, T, shared<T>, _Types...>
{
    using type = shared<T>;
};

template<typename StateData, typename T, typename... _Types>
struct find_container<StateData, T const, shared<T>, _Types...>
{
    using type = shared<T>;
};

template<typename StateData, typename T, typename... _Types>
struct find_container<StateData, T const, shared<T*>, _Types...>
{
    using type = shared<T>;
};

template<typename StateData, typename T, typename... _Types>
struct find_container<StateData, T, shared<T*>, _Types...>
{
    using type = shared<T*>;
};

template<typename StateData, typename T, typename... _Types>
struct find_container<StateData, id, T, _Types...>
{
    using type = StateData;
};

template<typename StateData, typename T, typename... _Types>
struct find_container<StateData, id const, T, _Types...>
{
    using type = StateData;
};
template<typename T, typename StateData, typename X>
struct type_aliases;

template<typename T, typename StateData, typename... Types>
struct type_aliases<T, StateData, ::shed::internal::type_list<Types...>>
{
    using container = typename internal::
        find_container<StateData, typename std::remove_reference<T>::type, Types...>::type;
    using stored = typename container::value_type;
};

template<typename T, typename Table>
using column = typename internal::
    type_aliases<T, typename Table::state_data, typename Table::column_list>::container;

template<typename State, typename Table>
constexpr size_t state_id()
{
    return ::etl::type_list_index_of_type<typename Table::state_list::TL_type, State>::value;
}

inline bool all_args_set() { return true; }

template<typename T, typename... Args>
bool all_args_set(T const v, Args... args)
{
    return v && all_args_set(args...);
}

inline bool all_args_cmp() { return false; }

template<typename T, typename... Args>
bool all_args_cmp(T const a, T const b, Args... args)
{
    if (b < a)
    {
        return true;
    }
    if (a < b)
    {
        return false;
    }
    return all_args_cmp(args...);
}

template<typename ParamT>
struct ParamConvert
{
    template<typename T>
    static T& to_param_type(T& t)
    {
        return t;
    }

    static inline id to_param_type(id const t) { return t; }

    template<typename T>
    static T& to_param_type(T* const t)
    {
        return *t;
    }

    template<typename T>
    static bool is_set(T const&)
    {
        return true;
    }

    template<typename T>
    static bool is_set(T* const t)
    {
        return t != nullptr;
    }
};

template<typename ParamT>
struct ParamConvert<ParamT**>
{
    template<typename T>
    static T** to_param_type(T*& t)
    {
        return &t;
    }

    template<typename T>
    static bool is_set(T* const)
    {
        return true;
    }
};

template<typename ParamT>
struct ParamConvert<ParamT*&>
{
    static ParamT*& to_param_type(ParamT*& t) { return t; }

    static bool is_set(ParamT* const) { return true; }
};

template<typename Table>
using raw_table_t = typename std::remove_const<typename std::remove_reference<Table>::type>::type;

template<typename Arg, typename Table>
using selected_column_t = typename std::conditional<
    std::is_const<typename std::remove_reference<Table>::type>::value,
    internal::column<Arg, raw_table_t<Table>> const,
    internal::column<Arg, raw_table_t<Table>>>::type;

template<typename Arg, typename Table>
selected_column_t<Arg, Table>& select_column(Table& table)
{
    using Column = selected_column_t<Arg, Table>;
    return *static_cast<Column*>(&table.columns);
}

template<typename TL>
struct type_list_recurse
{
    using recurse = type_list_recurse<typename TL::tail>;

    template<typename Table>
    static bool all_set(Table& table, size_t const i)
    {
        return ParamConvert<typename TL::head::value_type>::is_set(
                   (*static_cast<internal::column<typename TL::head::value_type, Table>*>(
                       &table.columns))[i])
               && recurse::all_set(table, i);
    }

    template<typename Table>
    static bool less_than(Table& table, size_t const a, size_t const b)
    {
        auto const f = typename TL::head();
        auto const& cmpType
            = *static_cast<internal::column<typename TL::head::value_type, Table>*>(&table.columns);
        auto const aType = ParamConvert<typename TL::head::value_type>::to_param_type(cmpType[a]);
        auto const bType = ParamConvert<typename TL::head::value_type>::to_param_type(cmpType[b]);

        if (f(bType, aType))
        {
            return true;
        }
        if (f(aType, bType))
        {
            return false;
        }
        return recurse::less_than(table, a, b);
    }
};

template<>
struct type_list_recurse<::etl::type_list<>>
{
    template<typename Table>
    static bool all_set(Table const&, size_t const)
    {
        return true;
    }

    template<typename Table>
    static bool less_than(Table const&, size_t const, size_t const)
    {
        return false;
    }
};

template<typename R, typename F, typename Table, typename... Args>
struct system_func
{
    static bool call(F& f, Table& table, size_t const i)
    {
        if (all_args_set(ParamConvert<Args>::is_set(select_column<Args>(table)[i])...))
        {
            return f(ParamConvert<Args>::to_param_type(select_column<Args>(table)[i])...);
        }
        return true;
    }

    template<typename T = Table, typename std::enable_if<!std::is_const<T>::value, int>::type = 0>
    static bool call(F& f, Table const& table, size_t const i)
    {
        if (all_args_set(ParamConvert<Args>::is_set(select_column<Args>(table)[i])...))
        {
            return f(ParamConvert<Args>::to_param_type(select_column<Args>(table)[i])...);
        }
        return true;
    }
};

// suppress false-positive gcc compiler warning
// (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=56958)
#if defined(__GNUC__) && (!defined(__llvm__)) && (!defined(_MSC_VER)) \
    && (!defined(__INTEL_COMPILER))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-parameter"
#endif // defined(__GNUC__) && !defined(__llvm__) && !defined(_MSC_VER) &&
       // !defined(__INTEL_COMPILER)
template<typename F, typename Table, typename... Args>
struct system_func<void, F, Table, Args...>
{
    static bool call(F& f, Table& table, size_t const i)
    {
        if (all_args_set(ParamConvert<Args>::is_set(select_column<Args>(table)[i])...))
        {
            f(ParamConvert<Args>::to_param_type(select_column<Args>(table)[i])...);
        }
        return true;
    }

    template<typename T = Table, typename std::enable_if<!std::is_const<T>::value, int>::type = 0>
    static bool call(F& f, Table const& table, size_t const i)
    {
        if (all_args_set(ParamConvert<Args>::is_set(select_column<Args>(table)[i])...))
        {
            f(ParamConvert<Args>::to_param_type(select_column<Args>(table)[i])...);
        }
        return true;
    }
};

inline move_op to_op(move_op const o) { return o; }

inline move_op to_op(bool const b) { return skip_if(b); }

// suppress false-positive gcc compiler warning (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=56958)
#if defined(__GNUC__) && (!defined(__llvm__)) && (!defined(_MSC_VER)) \
    && (!defined(__INTEL_COMPILER))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-parameter"
#endif // defined(__GNUC__) && !defined(__llvm__) && !defined(_MSC_VER) &&
       // !defined(__INTEL_COMPILER)
template<typename R, typename F, typename Table, typename I, typename... Args>
move_op call_system_func_ret(F& f, Table& table, I const i)
{
    if (all_args_set(ParamConvert<Args>::is_set(select_column<Args>(table)[i])...))
    {
        return to_op(f(ParamConvert<Args>::to_param_type(select_column<Args>(table)[i])...));
    }
    return move_op::SKIP;
}
#if defined(__GNUC__) && (!defined(__llvm__)) && (!defined(_MSC_VER)) \
    && (!defined(__INTEL_COMPILER))
#pragma GCC diagnostic pop
#endif // defined(__GNUC__) && !defined(__llvm__) && !defined(_MSC_VER) &&
       // !defined(__INTEL_COMPILER)

void move_while_impl(
    internal::multi_list::idx_type* begin,
    internal::multi_list::idx_type* end,
    multi_list* ml,
    size_t dst,
    ::etl::delegate<bool(size_t, size_t)> pred,
    ::etl::delegate<move_op(size_t)> csfr);

template<typename R, typename... Args>
struct SelectColumns
{
    template<typename Table, typename Q>
    static void for_each(Table& table, Q& q, size_t const src)
    {
        static_cast<typename Table::state_data*>(&table.columns)
            ->ml->in_bucket(src)
            .iter(
                [&q, &table](size_t const i) -> bool
                { return internal::system_func<R, Q, Table, Args...>::call(q, table, i); });
    }

    template<typename Table, typename Q>
    static void for_each_not(Table& table, Q& q, size_t const src)
    {
        static_cast<typename Table::state_data*>(&table.columns)
            ->ml->not_in_bucket(src)
            .iter(
                [&q, &table](size_t const i) -> bool
                { return internal::system_func<R, Q, Table, Args...>::call(q, table, i); });
    }

    template<typename Table, typename Q>
    static void for_each_not(Table const& table, Q& q, size_t const src)
    {
        static_cast<typename Table::state_data const*>(&table.columns)
            ->ml->not_in_bucket(src)
            .iter(
                [&q, &table](size_t const i) -> bool
                { return internal::system_func<R, Q, Table, Args...>::call(q, table, i); });
    }

    template<typename Table, typename Q>
    static void move(Table& table, Q& q, size_t const src, size_t const dst)
    {
        (void)static_cast<typename Table::state_data*>(&table.columns)
            ->ml->move_if(
                src,
                dst,
                [&q, &table](size_t const i) -> move_op {
                    return call_system_func_ret<::shed::move_op, Q, Table, size_t, Args...>(
                        q, table, i);
                });
    }

    template<typename Cmp, typename Table, typename Q, typename It>
    static void move_while(Table& table, Q& q, size_t const dst, It& it)
    {
        using tr = type_list_recurse<Cmp>;

        auto& ml  = static_cast<typename Table::state_data*>(&table.columns)->ml;
        auto csfr = [&q, &table](size_t const i) -> move_op
        { return call_system_func_ret<::shed::move_op, Q, Table, size_t, Args...>(q, table, i); };

        auto pred = [&table](size_t const a, size_t const b) -> bool
        { return tr::less_than(table, a, b); };

        auto const scratch = table.alloc_scratch();
        auto const begin   = scratch._scratch.begin();
        auto end           = begin;
        auto f             = [&table, &end](size_t const i) -> bool
        {
            if (tr::all_set(table, i))
            {
                *end = static_cast<internal::multi_list::idx_type>(i);
                ++end;
            }
            return true;
        };
        it.iter(f);

        ::shed::internal::move_while_impl(begin, end, ml, dst, pred, csfr);
    }

    template<typename Table, typename Q>
    static void move_from_any(Table& table, Q& q, size_t const dst)
    {
        auto& ml      = static_cast<typename Table::state_data*>(&table.columns)->ml;
        auto const it = ml->not_in_buckets(internal::state_id<internal::FREE, Table>(), dst);
        auto f        = [&q, &table, &dst, &ml](size_t const i) -> bool
        {
            auto const r
                = call_system_func_ret<::shed::move_op, Q, Table, size_t, Args...>(q, table, i);
            if ((r == move_op::MOVE) || (r == move_op::MOVE_DONE))
            {
                ml->move_idx(i, dst);
            }
            return !((r == move_op::SKIP_DONE) || (r == move_op::MOVE_DONE));
        };
        it.iter(f);
    }

    template<typename Table, typename Q>
    static void for_one(Table& table, Q& q, size_t const i)
    {
        (void)system_func<R, Q, Table, Args...>::call(q, table, i);
    }
};

template<typename R, typename... Args>
struct SelectColumns<R(Args...)> : SelectColumns<R, Args...>
{};

template<typename R, typename Cls, typename... Args>
struct SelectColumns<R (Cls::*)(Args...) const> : SelectColumns<R, Args...>
{};

template<typename R, typename Cls, typename... Args>
struct SelectColumns<R (Cls::*)(Args...)> : SelectColumns<R, Args...>
{};

struct reset_row
{
    size_t idx;

    template<typename T>
    void operator()(::shed::shared<T>&)
    {}

    template<typename T>
    void operator()(::shed::internal::state_data<T>&)
    {}

    template<typename T>
    void operator()(::shed::column<T>& v) const
    {
        v[idx].~T();
    }
};

struct init_row
{
    size_t idx;

    template<typename T>
    void operator()(::shed::column<T>& v) const
    {
        new (&v[idx]) T();
    }

    template<typename T>
    void operator()(::shed::shared<T>&)
    {}

    template<typename T>
    void operator()(::shed::internal::state_data<T>&)
    {}
};

} // namespace internal
} // namespace shed
