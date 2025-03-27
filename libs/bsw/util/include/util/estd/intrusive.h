// Copyright 2024 Accenture.

#pragma once
#include <utility>

namespace util
{
namespace estd
{
/// \cond INTERNAL
namespace internal
{
template<template<typename> class Container, template<typename> class Node, typename NodeT>
struct intrusive;
}

/// \endcond

/**
 * A helper to make it possible to use unmodified types with intrusive containers.
 *
 * \section intrusive_usage Usage example
 * \code{.cpp}
 * // Given a type Subject, this will create type aliases for both
 * // the node as well as the corresponding list type
 * typedef util::estd::intrusive<::estd::forward_list, ::estd::forward_list_node> intrusive_list;
 * typedef intrusive_list::of<Subject>::container SubjectList;
 * typedef intrusive_list::of<Subject>::node SubjectNode;
 * \endcode
 *
 */
template<template<typename> class Container, template<typename> class Node>
struct intrusive
{
    template<typename T>
    struct of
    {
        using node      = typename internal::intrusive<Container, Node, T>::template node<T>;
        using container = typename internal::intrusive<Container, Node, T>::container;
    };
};

/// \cond INTERNAL
namespace internal
{
template<template<typename> class Container, template<typename> class Node, typename NodeT>
struct intrusive
{
    template<typename ValueT = NodeT>
    class node
    : public Node<node<NodeT>>
    , public ValueT
    {
        node(node const&)            = delete;
        node& operator=(node const&) = delete;

    public:
        using type = ValueT;

        node& operator=(ValueT const& other)
        {
            *static_cast<ValueT*>(this) = other;
            return *this;
        }

        template<typename... Args>
        explicit node(Args&&... args) : ValueT(std::forward<Args>(args)...)
        {}
    };

    using container = Container<node<NodeT>>;
};
} // namespace internal

/// \endcond

} /* namespace estd */
} /* namespace util */
