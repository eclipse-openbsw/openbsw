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

namespace util
{
namespace types
{
/**
 * Implementation of a typesafe enum.
 */
template<class Values, class Underlying = typename Values::type>
class Enum : public Values
{
protected:
    using type       = typename Values::type;
    using value_type = Underlying;

    Underlying _value;

public:
    static Enum fromUnderlying(Underlying v);

    Enum();

    Enum(type v);

    Enum(Enum const& e);

    Enum& operator=(Enum const& e)
    {
        if (this != &e)
        {
            _value = e._value;
        }
        return *this;
    }

    Underlying value() const { return _value; }

    friend bool operator==(Enum const& lhs, Enum const& rhs) { return lhs._value == rhs._value; }

    friend bool operator!=(Enum const& lhs, Enum const& rhs) { return lhs._value != rhs._value; }

    friend bool operator<(Enum const& lhs, Enum const& rhs) { return lhs._value < rhs._value; }

    friend bool operator<=(Enum const& lhs, Enum const& rhs) { return lhs._value <= rhs._value; }

    friend bool operator>(Enum const& lhs, Enum const& rhs) { return lhs._value > rhs._value; }

    friend bool operator>=(Enum const& lhs, Enum const& rhs) { return lhs._value >= rhs._value; }
};

/*
 * inline implementation
 */
// static
template<class Values, class Underlying>
inline Enum<Values, Underlying> Enum<Values, Underlying>::fromUnderlying(Underlying const v)
{
    return Enum(static_cast<type>(v));
}

template<class Values, class Underlying>
inline Enum<Values, Underlying>::Enum() : Values(), _value()
{}

template<class Values, class Underlying>
inline

    Enum<Values, Underlying>::Enum(type const v)
: Values(), _value(v)
{}

template<class Values, class Underlying>
inline Enum<Values, Underlying>::Enum(Enum const& e) : Values(), _value(e._value)
{}

} /* namespace types */
} /* namespace util */
