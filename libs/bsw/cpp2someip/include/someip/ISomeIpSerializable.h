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

#include <cstdint>

namespace someip
{
class SomeIpParser;
class SomeIpSerializer;

/**
 * Interface for serializable SOME/IP data types. The SOME/IP code generator
 * will generate classes that extend this interface.
 */
class ISomeIpSerializable
{
protected:
    ISomeIpSerializable() = default;

public:
    ISomeIpSerializable(ISomeIpSerializable const&)            = delete;
    ISomeIpSerializable& operator=(ISomeIpSerializable const&) = delete;

    virtual ~ISomeIpSerializable() = default;

    /**
     * Pure virtual function for serializing the content
     * of the data type to the specified serializer object.
     *
     * \param serializer The serializer object used for writing this object to bytes.
     */
    virtual void serializeToArray(SomeIpSerializer& serializer) const = 0;

    /**
     * Pure virtual function that parses the content of
     * this object from the specified parser.
     *
     * \param parser The parser object used for reading this object from bytes.
     */
    virtual void parseFromArray(SomeIpParser& parser) = 0;

    /**
     * Pure virtual function that returns the number of bytes
     * required for serializing this data type.
     */
    virtual uint32_t getSize() const = 0;
};

} // namespace someip
