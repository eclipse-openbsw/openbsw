// Copyright 2024 Accenture.

#ifndef GUARD_31FBB82D_9B99_446D_BC36_43975F78F3F0
#define GUARD_31FBB82D_9B99_446D_BC36_43975F78F3F0

#include <platform/estdint.h>

#include <tuple>

namespace estd
{
namespace internal
{
template<size_t Index, class Search, class First, class... Types>
struct get_helper
{
    using type                    = typename get_helper<Index + 1, Search, Types...>::type;
    static constexpr size_t index = Index;
};

template<size_t Index, class Search, class... Types>
struct get_helper<Index, Search, Search, Types...>
{
    using type                    = get_helper;
    static constexpr size_t index = Index;
};

// The following two seemingly pointless functions are required to use C++11
// parameter pack expansion.
//
// It is not possible to write:
// f(args)...;
//
// Pack expansion only works in the context of a parameter list:
// do_nothing(f(args)...);
//
// This still does not work in this case because f returns void and void cannot
// be a parameter type. So an extra (inlined) indirection is used to make the return type
// void* instead:
// do_nothing(ignore_return(f,args))...);

template<typename... Types>
void do_nothing(Types const...)
{}

template<typename F, typename T>
ESR_NO_INLINE void* ignore_return(F&& f, T&& t)
{
    f(t);
    return nullptr;
}

} // namespace internal

// This is std::get by-type backported from c++14.
// Can be removed as soon as C++14 can be required
template<class T, class... Types>
T& get(std::tuple<Types...>& tuple)
{
    return std::get<internal::get_helper<0, T, Types...>::type::index>(tuple);
}

template<class T, class... Types>
T const& get(std::tuple<Types...> const& tuple)
{
    return std::get<internal::get_helper<0, T, Types...>::type::index>(tuple);
}

template<typename F, typename... Types>
void for_each(std::tuple<Types...>& args, F&& f)
{
    internal::do_nothing(internal::ignore_return(f, ::estd::get<Types>(args))...);
}

template<typename F, typename... Types>
void for_each(std::tuple<Types...> const& args, F&& f)
{
    internal::do_nothing(internal::ignore_return(f, ::estd::get<Types>(args))...);
}

} // namespace estd

#endif
