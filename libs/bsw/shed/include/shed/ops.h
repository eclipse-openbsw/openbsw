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

#include "shed/ops_internal.h"   // IWYU pragma: export
#include "shed/table_internal.h" // IWYU pragma: export

#include <type_traits>

namespace shed
{
template<typename Src, typename Table, typename F>
void for_each(Table& table, F* f)
{
    return internal::SelectColumns<F>::for_each(table, f, internal::state_id<Src, Table>());
}

template<typename Src, typename Table, typename F>
void for_each(Table& table, F const& f)
{
    return internal::SelectColumns<decltype(&F::operator())>::for_each(
        table, f, internal::state_id<Src, Table>());
}

template<typename Table, typename F>
void for_each(Table& table, F* f)
{
    return internal::SelectColumns<F>::for_each_not(
        table, f, internal::state_id<internal::FREE, Table>());
}

template<typename Table, typename In, typename F>
void for_each_in(Table& table, In const& in, F* f)
{
    for (auto&& i : in)
    {
        internal::SelectColumns<F>::for_one(table, f, i);
    }
}

template<typename Table, typename In, typename F>
void for_each_in(Table& table, In const& in, F const& f)
{
    for (auto&& i : in)
    {
        internal::SelectColumns<decltype(&F::operator())>::for_one(table, f, i);
    }
}

template<typename Table, typename F>
void for_each(Table& table, F const& f)
{
    return internal::SelectColumns<decltype(&F::operator())>::for_each_not(
        table, f, internal::state_id<internal::FREE, Table>());
}

template<typename Table, typename F>
void for_each(Table const& table, F const& f)
{
    return internal::SelectColumns<decltype(&F::operator())>::for_each_not(
        table, f, internal::state_id<internal::FREE, Table>());
}

template<typename Cmp>
struct reverse
{
    using value_type = typename Cmp::value_type;

    bool operator()(value_type const& a, value_type const& b) const { return Cmp()(b, a); }
};

template<typename Dst>
struct move_to
{
    template<typename Table, typename F>
    move_to(Table& table, F const& f)
    {
        constexpr auto dst = internal::state_id<Dst, Table>();
        internal::SelectColumns<decltype(&F::operator())>::move_from_any(table, f, dst);
    }

    template<typename Table, typename F>
    move_to(Table& table, F* f)
    {
        constexpr auto dst = internal::state_id<Dst, Table>();
        internal::SelectColumns<F>::move_from_any(table, f, dst);
    }

    template<typename Table>
    static void by_id(Table& table, id const i)
    {
        if (valid(table, i))
        {
            static_cast<typename Table::state_data*>(&table.columns)
                ->ml->move_idx(i, internal::state_id<Dst, Table>());
        }
    }

    template<typename Src, typename Table>
    static void all(Table& table)
    {
        from<Src>(table, []() -> move_op { return move_op::MOVE; });
    }

    template<typename Src>
    struct from
    {
        template<typename Table, typename F>
        from(Table& table, F const& f)
        {
            internal::SelectColumns<decltype(&F::operator())>::move(
                table, f, internal::state_id<Src, Table>(), internal::state_id<Dst, Table>());
        }

        template<typename Table, typename F>
        from(Table& table, F* f)
        {
            internal::SelectColumns<F>::move(
                table, f, internal::state_id<Src, Table>(), internal::state_id<Dst, Table>());
        }

        template<typename... Cmp>
        struct ordered
        {
            template<typename Table, typename F>
            ordered(Table& table, F* f)
            {
                constexpr auto src = internal::state_id<Src, Table>();
                constexpr auto dst = internal::state_id<Dst, Table>();
                auto const it
                    = static_cast<typename Table::state_data*>(&table.columns)->ml->in_bucket(src);
                internal::SelectColumns<F>::template move_while<::etl::type_list<Cmp...>>(
                    table, f, dst, it);
            }

            template<typename Table, typename F>
            ordered(Table& table, F const& f)
            {
                constexpr auto src = internal::state_id<Src, Table>();
                constexpr auto dst = internal::state_id<Dst, Table>();
                auto const it
                    = static_cast<typename Table::state_data*>(&table.columns)->ml->in_bucket(src);
                internal::SelectColumns<decltype(&F::operator())>::template move_while<
                    ::etl::type_list<Cmp...>>(table, f, dst, it);
            }

            template<typename Table, typename F>
            ordered(Table& table, F& f)
            {
                constexpr auto src = internal::state_id<Src, Table>();
                constexpr auto dst = internal::state_id<Dst, Table>();
                auto const it
                    = static_cast<typename Table::state_data*>(&table.columns)->ml->in_bucket(src);
                internal::SelectColumns<decltype(&F::operator())>::template move_while<
                    ::etl::type_list<Cmp...>>(table, f, dst, it);
            }
        };
    };

    template<typename... Cmp>
    struct ordered
    {
        template<typename Table, typename F>
        ordered(Table& table, F* f)
        {
            constexpr auto src = internal::state_id<internal::FREE, Table>();
            constexpr auto dst = internal::state_id<Dst, Table>();
            auto const it      = static_cast<typename Table::state_data*>(&table.columns)
                                ->ml->not_in_buckets(src, dst);
            internal::SelectColumns<F>::template move_while<::etl::type_list<Cmp...>>(
                table, f, dst, it);
        }

        template<typename Table, typename F>
        ordered(Table& table, F const& f)
        {
            constexpr auto src = internal::state_id<internal::FREE, Table>();
            constexpr auto dst = internal::state_id<Dst, Table>();
            auto const it      = static_cast<typename Table::state_data*>(&table.columns)
                                ->ml->not_in_buckets(src, dst);
            internal::SelectColumns<decltype(&F::operator())>::template move_while<
                ::etl::type_list<Cmp...>>(table, f, dst, it);
        }
    };
};

template<typename Table>
void drop(Table& table, id i)
{
    ft_for_each(table.columns, internal::reset_row{i});
    static_cast<typename Table::state_data*>(&table.columns)->generations[i] = 0;
    static_cast<typename Table::state_data*>(&table.columns)
        ->ml->move_idx(i, internal::state_id<internal::FREE, Table>());
}

template<typename Src, typename Table>
size_t drop(Table& table)
{
    return static_cast<typename Table::state_data*>(&table.columns)
        ->ml->move_if(
            internal::state_id<Src, Table>(),
            internal::state_id<internal::FREE, Table>(),
            [&table](size_t const idx) -> move_op
            {
                ft_for_each(table.columns, internal::reset_row{idx});
                static_cast<internal::column<id, Table>*>(&table.columns)->generations[idx] = 0;
                return move_op::MOVE;
            });
}

template<typename Dst, typename Table>
id insert(Table& table)
{
    auto& sd     = *static_cast<typename Table::state_data*>(&table.columns);
    auto const i = sd.ml->move_first(
        internal::state_id<internal::FREE, Table>(), internal::state_id<Dst, Table>());
    if (i == table.size())
    {
        return {};
    }
    sd.generations[i] = sd.generation;
    ++sd.generation;
    if (sd.generation == 0)
    {
        ++sd.generation;
    }
    ft_for_each(table.columns, internal::init_row{i});
    return sd[i];
}

template<typename Dst, typename Table, typename F>
id insert(Table& table, F const& f)
{
    auto const i = insert<Dst>(table);
    if (i.valid())
    {
        internal::SelectColumns<decltype(&F::operator())>::for_one(table, f, i);
    }
    return i;
}

template<typename Table, typename F>
void with(Table& table, id const i, F* f)
{
    if (valid(table, i))
    {
        internal::SelectColumns<F>::for_one(table, f, i);
    }
}

template<typename Table, typename F>
void with(Table& table, id const i, F const& f)
{
    if (valid(table, i))
    {
        internal::SelectColumns<decltype(&F::operator())>::for_one(table, f, i);
    }
}

template<typename Table, typename F>
void with(Table const& table, id const i, F const& f)
{
    if (valid(table, i))
    {
        internal::SelectColumns<decltype(&F::operator())>::for_one(table, f, i);
    }
}

template<typename Table>
bool valid(Table const& table, id const i)
{
    return (i.generation != 0)
           && (i.generation
               == static_cast<typename Table::state_data const*>(&table.columns)->generations[i]);
}

template<typename Table>
typename internal::multi_list::NotInBucket all(Table const& table)
{
    return static_cast<typename Table::state_data const*>(&table.columns)
        ->ml->not_in_bucket(internal::state_id<internal::FREE, Table>());
}

template<typename State, typename Table>
typename internal::multi_list::InBucket all(Table const& table)
{
    return static_cast<typename Table::state_data const*>(&table.columns)
        ->ml->in_bucket(internal::state_id<State, Table>());
}

template<typename T, typename Table>
internal::column<T, Table>& get(Table& table)
{
    return *static_cast<internal::column<T, Table>*>(&table.columns);
}

template<typename T, typename Table>
internal::column<T, Table> const& get(Table const& table)
{
    return *static_cast<internal::column<T, Table> const*>(&table.columns);
}

template<typename T, typename Table>
id id_of(Table const& table, T const* ptr)
{
    using column_type = internal::column<T const, Table>;
    static_assert(
        std::is_same<column_type, ::shed::column<typename column_type::value_type>>::value,
        "shed::id_of only supports values stored in shed::column");

    if (ptr == nullptr)
    {
        return {};
    }

    auto const data         = get<T const>(table).data();
    auto const* const begin = data.data();
    auto const* const end   = begin + data.size();
    if ((ptr < begin) || (ptr >= end))
    {
        return {};
    }

    auto const index = static_cast<size_t>(ptr - begin);
    auto const rowId
        = static_cast<typename Table::state_data const*>(&table.columns)->operator[](index);
    return valid(table, rowId) ? rowId : ::shed::id{};
}

} // namespace shed
