// Copyright 2024 Accenture.

#ifndef GUARD_B16DC1F5_FC61_4998_8D44_689E59FB9B3A
#define GUARD_B16DC1F5_FC61_4998_8D44_689E59FB9B3A

#include "util/estd/signal.h"

#include <estd/functional.h>

namespace util
{
namespace estd
{
template<class T>
struct default_compare
{
    static bool equal(T const& a, T const& b) { return a == b; }
};

/**
 * Implementation of the observer pattern (https://en.wikipedia.org/wiki/Observer_pattern)
 *
 * An instance of this class maintains a list of observers that are functions to be called
 * whenever the observed value changes.
 */
template<class T, class Compare = default_compare<T>>
class observable
{
    using signal_t = ::util::estd::signal<::estd::function<void(T)>>;

public:
    using value_type = T;

    using observer = typename signal_t::slot;

    signal_t signal;

    class transaction
    {
        observable& _o;
        T _new_value;

    public:
        using value_type = T;

        explicit transaction(observable&);
        ~transaction();

        transaction& operator=(T const& rhs);

        /**
         * Sets the corresponding oberservers value to the one held in this transaction.
         * If they where different, true is returned. Otherwise false.
         */
        bool commit();

        T& get();

        T* operator->();
    };

    observable();
    explicit observable(T const& value);

    void add_observer(observer& o);
    void remove_observer(observer& o);

    void notify();

    observable& operator=(T const& rhs);
    operator T const&() const;
    T const& get() const;

    /**
     * Sets the wrapped variable to the new value "value" and calls all observers
     * when the value is different.
     * If the value changed true is returned. Otherwise false.
     */
    bool set(T const& value);

    transaction operator->();

private:
    T _value;
};

// implementation

template<class T, class Compare>
observable<T, Compare>::observable(T const& value) : signal(), _value(value)
{}

template<class T, class Compare>
observable<T, Compare>::observable() : observable(T())
{}

template<class T, class Compare>
void observable<T, Compare>::add_observer(observable<T, Compare>::observer& o)
{
    signal.connect(o);
}

template<class T, class Compare>
void observable<T, Compare>::remove_observer(observable<T, Compare>::observer& o)
{
    signal.disconnect(o);
}

template<class T, class Compare>
void observable<T, Compare>::notify()
{
    signal(_value);
}

template<class T, class Compare>
T const& observable<T, Compare>::get() const
{
    return _value;
}

template<class T, class Compare>
bool observable<T, Compare>::set(T const& value)
{
    if (!Compare::equal(_value, value))
    {
        _value = value;
        notify();
        return true;
    }
    return false;
}

template<class T, class Compare>
observable<T, Compare>& observable<T, Compare>::operator=(T const& rhs)
{
    (void)set(rhs);
    return *this;
}

template<class T, class Compare>
typename observable<T, Compare>::transaction&
observable<T, Compare>::transaction::operator=(T const& rhs)
{
    _new_value = rhs;
    return *this;
}

template<class T, class Compare>
observable<T, Compare>::operator T const&() const
{
    return _value;
}

template<class T, class Compare>
typename observable<T, Compare>::transaction observable<T, Compare>::operator->()
{
    return transaction(*this);
}

template<class T, class Compare>
T* observable<T, Compare>::transaction::operator->()
{
    return &_new_value;
}

template<class T, class Compare>
observable<T, Compare>::transaction::transaction(observable<T, Compare>& o)
: _o(o), _new_value(o._value)
{}

template<class T, class Compare>
observable<T, Compare>::transaction::~transaction()
{
    (void)_o.set(_new_value);
}

template<class T, class Compare>
bool observable<T, Compare>::transaction::commit()
{
    return _o.set(_new_value);
}

template<class T, class Compare>
T& observable<T, Compare>::transaction::get()
{
    return _new_value;
}

} /* namespace estd */
} /* namespace util */

#endif /* GUARD_B16DC1F5_FC61_4998_8D44_689E59FB9B3A */
