/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "someip/ISomeIpSerializable.h"
#include "someip/SomeIpParser.h"
#include "someip/SomeIpSerializer.h"

#include <util/types/Enum.h>

namespace someip
{
/**
 * A base class object for all generated enum types.
 *
 * \tparam Values The possible enum values for this class.
 * \tparam Underlying The underlying type that can store all enum elements in the Values type.
 */
template<class Values, class Underlying>
class EnumSerializable
: public ::util::types::Enum<Values, Underlying>
, public ISomeIpSerializable
{
protected:
    /** Helper typedef for accessing the underlying Enum types. */
    using base_type = ::util::types::Enum<Values, Underlying>;

public:
    using enum_type = typename base_type::type;

    /** The underlying type for this enum. */
    using value_type = typename base_type::value_type;

    /** Initializes this instance to a default value */
    EnumSerializable();

    /** Initialize this instance from the underlying enum */
    explicit EnumSerializable(enum_type value);

    /** Initialize this instance from a uint8_t */
    explicit EnumSerializable(value_type value);

    /** A simple copy constructor */
    EnumSerializable(EnumSerializable<Values, Underlying> const& other);

    /** A simple assignment operator */
    EnumSerializable<Values, Underlying>&
    operator=(EnumSerializable<Values, Underlying> const& other);

    /** Serializes this enum to a byte array. */
    void serializeToArray(SomeIpSerializer& serializer) const override;

    /** Parses this enum from a byte array. */
    void parseFromArray(SomeIpParser& parser) override;

    /** Returns the number of bytes necessary to save this enum in a byte array. */
    uint32_t getSize() const override;
};

/*
 * inline implementation
 */
template<class Values, class Underlying>
inline EnumSerializable<Values, Underlying>::EnumSerializable() : base_type()
{}

template<class Values, class Underlying>
inline EnumSerializable<Values, Underlying>::EnumSerializable(
    EnumSerializable<Values, Underlying> const& other)
: base_type(other), ISomeIpSerializable()
{}

template<class Values, class Underlying>
inline EnumSerializable<Values, Underlying>::EnumSerializable(enum_type const value)
: base_type(value)
{}

template<class Values, class Underlying>
inline EnumSerializable<Values, Underlying>::EnumSerializable(value_type const value)
: base_type(base_type::fromUnderlying(value))
{}

template<class Values, class Underlying>
inline EnumSerializable<Values, Underlying>&
EnumSerializable<Values, Underlying>::operator=(EnumSerializable const& other)
{
    base_type::operator=(other);
    return *this;
}

template<class Values, class Underlying>
inline void
EnumSerializable<Values, Underlying>::serializeToArray(SomeIpSerializer& serializer) const
{
    serializer << base_type::_value;
}

template<class Values, class Underlying>
inline void EnumSerializable<Values, Underlying>::parseFromArray(SomeIpParser& parser)
{
    parser >> base_type::_value;
}

template<class Values, class Underlying>
inline uint32_t EnumSerializable<Values, Underlying>::getSize() const
{
    return static_cast<uint32_t>(sizeof(base_type::_value));
}

} // namespace someip
