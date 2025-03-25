// Copyright 2024 Accenture.

#pragma once

#include "util/estd/intrusive.h"

#include <estd/forward_list.h>
#include <estd/functional.h>

namespace util
{
namespace estd
{
/**
 * Implements the pattern of broadcasting a function call to a set of slots, who have
 * subscribed to it.
 */
template<class F>
class signal
{
public:
    using slot_function = F;

    using intrusive_list = util::estd::intrusive<::estd::forward_list, ::estd::forward_list_node>;

    using slot_list = typename intrusive_list::of<slot_function>::container;
    using slot      = typename intrusive_list::of<slot_function>::node;

    void connect(slot& s);

    void disconnect(slot& s);

    template<class P1>
    void operator()(const P1& p1);

    template<class P1, class P2>
    void operator()(const P1& p1, const P2& p2);

    template<class P1, class P2, class P3>
    void operator()(const P1& p1, const P2& p2, const P3& p3);

    template<class P1, class P2, class P3, class P4>
    void operator()(const P1& p1, const P2& p2, const P3& p3, const P4& p4);

private:
    slot_list _slots;
};

// Implementation
template<class F>
inline void signal<F>::connect(signal<F>::slot& s)
{
    _slots.push_front(s);
}

template<class F>
inline void signal<F>::disconnect(signal<F>::slot& s)
{
    _slots.remove(s);
}

template<class F>
template<typename P1>
inline void signal<F>::operator()(const P1& p1)
{
    typename slot_list::iterator const end = _slots.end();
    typename slot_list::iterator it        = _slots.begin();

    while (it != end)
    {
        (*it)(p1);
        ++it;
    }
}

template<class F>
template<class P1, class P2>
inline void signal<F>::operator()(const P1& p1, const P2& p2)
{
    typename slot_list::iterator const end = _slots.end();
    typename slot_list::iterator it        = _slots.begin();

    while (it != end)
    {
        (*it)(p1, p2);
        ++it;
    }
}

template<class F>
template<class P1, class P2, class P3>
inline void signal<F>::operator()(const P1& p1, const P2& p2, const P3& p3)
{
    typename slot_list::iterator const end = _slots.end();
    typename slot_list::iterator it        = _slots.begin();

    while (it != end)
    {
        (*it)(p1, p2, p3);
        ++it;
    }
}

template<class F>
template<class P1, class P2, class P3, class P4>
inline void signal<F>::operator()(const P1& p1, const P2& p2, const P3& p3, const P4& p4)
{
    typename slot_list::iterator const end = _slots.end();
    typename slot_list::iterator it        = _slots.begin();

    while (it != end)
    {
        (*it)(p1, p2, p3, p4);
        ++it;
    }
}

} // namespace estd
} // namespace util

#define SIGNAL(_SIGNATURE) ::util::estd::signal<::estd::function<void _SIGNATURE>>

#define SLOT_BASE(_FUNC, _SIGNATURE)                                                 \
    template<class T, class S>                                                       \
    inline void connect_helper_##_FUNC(T&, S& signal)                                \
    {                                                                                \
        slot_##_FUNC = SIGNAL(_SIGNATURE)::slot::type::create<T, &T ::_FUNC>(*this); \
        signal.connect(slot_##_FUNC);                                                \
    }                                                                                \
    template<class S>                                                                \
    inline void connect_##_FUNC(S& signal)                                           \
    {                                                                                \
        connect_helper_##_FUNC(*this, signal);                                       \
    }                                                                                \
    template<class S>                                                                \
    inline void disconnect_##_FUNC(S& signal)                                        \
    {                                                                                \
        signal.disconnect(slot_##_FUNC);                                             \
    }                                                                                \
    SIGNAL(_SIGNATURE)::slot slot_##_FUNC;

#define SLOT(_FUNC, _SIGNATURE) SLOT_BASE(_FUNC, _SIGNATURE) void _FUNC _SIGNATURE

#define MOCK_SLOT1(_FUNC, _SIG) SLOT_BASE(_FUNC, _SIG) MOCK_METHOD1(_FUNC, void _SIG)
#define MOCK_SLOT2(_FUNC, _SIG) SLOT_BASE(_FUNC, _SIG) MOCK_METHOD2(_FUNC, void _SIG)
#define MOCK_SLOT3(_FUNC, _SIG) SLOT_BASE(_FUNC, _SIG) MOCK_METHOD3(_FUNC, void _SIG)
#define MOCK_SLOT4(_FUNC, _SIG) SLOT_BASE(_FUNC, _SIG) MOCK_METHOD4(_FUNC, void _SIG)

