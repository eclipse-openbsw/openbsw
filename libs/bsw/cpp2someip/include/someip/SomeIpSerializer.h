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
#include "someip/SomeIpStreamer.h"

#include "someip/endian_helpers.h"
#include <etl/span.h>
#include <cmath>

namespace someip
{
/**
 * The SomeIpSerializer class provides an easy way to serialize Some/IP
 * types into raw bytes. This class also provides a helper function for
 * writing out a length field. The class is created with a buffer to
 * ensure that buffer overwrites do not happen. If a serializer object
 * enters an error state all subsequent writes will be empty operations;
 * they will not cause other problems.
 *
 * \section SomeIpSerializer_example Usage example
 * \code{.cpp}
 * void serialize(::etl::span<uint8_t> target, const ISomeIpSerializable& obj)
 * {
 *     SomeIpSerializer serializer(target);
 *
 *     // create a promise so we can write the length when we are done
 *     {
 *         LengthHelper<uint8_t> promise(serializer);
 *
 *         size_t position = serializer.getCurrentPosition();
 *
 *         // write out the object
 *         serializer << obj;
 *
 *         // The length will be written when the LengthHelper goes out of scope.
 *     }
 *     // write more data
 *     serializer << uint32_t(42);
 * }
 * \endcode
 */
class SomeIpSerializer : public SomeIpStreamer
{
public:
    /**
     * Create a new SomeIpSerializer that uses the specified buffer.
     *
     * \param buffer The buffer the serializer will use to write bytes.
     */
    explicit SomeIpSerializer(::etl::span<uint8_t> buffer);

    /**
     * Returns the underlying payload as a const buffer.
     */
    ::etl::span<uint8_t const> getPayload() const { return _buffer.first(_currentPos); }

    size_t getCurrentPosition() const { return _currentPos; }

    /**
     * Returns how much more data can be written.
     */
    size_t bytesAvailable() const { return _buffer.size() - _currentPos; }

    /**
     * Writes out a bool value. The value is converted to a uint8_t and
     * then written to the byte buffer.
     *
     * \param item The boolean value to write.
     *
     * \note If the serializer is in an error state then no value will be written
     * to the buffer. If there is no space for the value then the serializer will
     * set to an error state.
     */
    void operator<<(bool const item) { write_byte(item); }

    /**
     * Writes out a char value. The value is converted to a uint8_t and
     * then written to the byte buffer.
     *
     * \param item The char value to write.
     *
     * \note If the serializer is in an error state then no value will be written
     * to the buffer. If there is no space for the value then the serializer will
     * set to an error state.
     */
    void operator<<(char const item) { write_byte(item); }

    /**
     * Writes out a uint8_t value.
     *
     * \param item The uint8_t value to write.
     *
     * \note If the serializer is in an error state then no value will be written
     * to the buffer. If there is no space for the value then the serializer will
     * set to an error state.
     */
    void operator<<(uint8_t const item) { write_byte(item); }

    /**
     * Writes out an int8_t value.
     *
     * \param item The int8_t value to write.
     *
     * \note If the serializer is in an error state then no value will be written
     * to the buffer. If there is no space for the value then the serializer will
     * set to an error state.
     */
    void operator<<(int8_t const item) { write_byte(item); }

    /**
     * Writes out a uint16_t value in the currently selected endianness.
     *
     * \param item The uint16_t value to write.
     *
     * \note If the serializer is in an error state then no value will be written
     * to the buffer. If there is no space for the value then the serializer will
     * set to an error state.
     */
    void operator<<(uint16_t const item) { write(item); }

    /**
     * Writes out an int16_t value in the currently selected endianness.
     *
     * \param item The int16_t value to write.
     *
     * \note If the serializer is in an error state then no value will be written
     * to the buffer. If there is no space for the value then the serializer will
     * set to an error state.
     */
    void operator<<(int16_t const item) { write(item); }

    /**
     * Writes out a uint32_t value in the currently selected endianness.
     *
     * \param item The uint32_t value to write.
     *
     * \note If the serializer is in an error state then no value will be written
     * to the buffer. If there is no space for the value then the serializer will
     * set to an error state.
     */
    void operator<<(uint32_t const item) { write(item); }

    /**
     * Writes out an int32_t value in the currently selected endianness.
     *
     * \param item The int32_t value to write.
     *
     * \note If the serializer is in an error state then no value will be written
     * to the buffer. If there is no space for the value then the serializer will
     * set to an error state.
     */
    void operator<<(int32_t const item) { write(item); }

    /**
     * Writes out a uint64_t value in the currently selected endianness.
     *
     * \param item The uint64_t value to write.
     *
     * \note If the serializer is in an error state then no value will be written
     * to the buffer. If there is no space for the value then the serializer will
     * set to an error state.
     */
    void operator<<(uint64_t const item) { write(item); }

    /**
     * Writes out an int64_t value in the currently selected endianness.
     *
     * \param item The int64_t value to write.
     *
     * \note If the serializer is in an error state then no value will be written
     * to the buffer. If there is no space for the value then the serializer will
     * set to an error state.
     */
    void operator<<(int64_t const item) { write(item); }

    /**
     * Writes out a float value according to the IEEE-754 standard.
     *
     * \param item The float value to write.
     *
     * \note If the serializer is in an error state then no value will be written
     * to the buffer. If there is no space for the value then the serializer will
     * set to an error state.
     */
    void operator<<(float_t item);

    /**
     * Writes out a double value according to the IEEE-754 standard.
     *
     * \param item The double value to write.
     *
     * \note If the serializer is in an error state then no value will be written
     * to the buffer. If there is no space for the value then the serializer will
     * set to an error state.
     */
    void operator<<(double_t item);

    /**
     * Writes out the ISomeIpSerializable object. This calls the
     * ISomeIpSerializable::serializeToArray method.
     *
     * \param item The serializable object to write.
     *
     * \note If the serializer is in an error state then no value will be written
     * to the buffer. If an error happens during the call to serializeToArray then
     * the serializer will be set to an error state. The data in the underlying buffer may
     * be corrupted and incomplete and should not be used.
     */
    void operator<<(ISomeIpSerializable const& item);

    void operator<<(::etl::span<uint8_t const> data);

    /**
     * Returns a buffer of a specified length at the current serializer position, where something
     * can be written later. This is used for writing sizes of dynamically sized types.
     *
     * In case the requested number of bytes is not available, an error is be set and empty slice is
     * returned.
     *
     * \param length The number of bytes that are requested.
     * \return Byte buffer at the current position.
     */
    ::etl::span<uint8_t> skipBytes(size_t const length)
    {
        size_t const start = _currentPos;

        if (!hasSpace(length))
        {
            return ::etl::span<uint8_t>();
        }

        _currentPos += length;
        return _buffer.subspan(start, length);
    }

    void writeTypeFieldSize(uint32_t fieldType);

private:
    template<typename T>
    void write(T const item)
    {
        if (hasSpace(sizeof(T)) && isGood())
        {
            if (_bigEndian)
            {
                ::someip::endian::write_be<T>(&_buffer[_currentPos], item);
            }
            else
            {
                ::someip::endian::write_le<T>(&_buffer[_currentPos], item);
            }
            _currentPos += sizeof(T);
        }
    }

    template<typename T>
    void write_byte(T const item)
    {
        if (hasSpace(1) && isGood())
        {
            _buffer[_currentPos] = static_cast<uint8_t>(item);
            _currentPos++;
        }
    }

    bool hasSpace(size_t const length)
    {
        if (_currentPos + length > _buffer.size())
        {
            _errorCode = ErrorCode::SOMEIP_ERROR;
            return false;
        }

        return true;
    }

    ::etl::span<uint8_t> _buffer;
    size_t _currentPos;
};

} // namespace someip
