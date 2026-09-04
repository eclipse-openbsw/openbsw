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

#include <cstdint>

#include <cmath>

namespace someip
{
/**
 * The SomeIpParser class provides an easy way to parse Some/IP
 * types from raw bytes. The class is created with a buffer to
 * ensure that buffer overwrites do not happen. If a parser object
 * enters an error state all subsequent reads will be empty operations;
 * they will not cause other problems.
 *
 * \section SomeIpParser_example Usage example
 * \code{.cpp}
 * void parser(::etl::span<const uint8_t> source, ISomeIpSerializable& obj)
 * {
 *     SomeIpParser parser(source);
 *
 *     // read the object
 *     parser >> obj;
 *
 *     // read some more data
 *     uint32_t value;
 *     parser >> value;
 * }
 * \endcode
 */
class SomeIpParser : public SomeIpStreamer
{
public:
    /**
     * Creates a new SomeIpParser using the specified constant byte buffer.
     *
     * \param buffer The buffer that holds the bytes that will be read.
     */
    explicit SomeIpParser(::etl::span<uint8_t const> buffer);

    /**
     * Reads a bool value from the current position in the buffer by reading
     * in a uint8_t and converting that to a bool.
     *
     * \param item The boolean value to read.
     *
     * \note If the parser is in an error state then no value will be read
     * from the buffer. If there is no space left in the buffer to read the value
     * then the parser will set to an error state.
     */
    void operator>>(bool& item) { read_byte(item); }

    /**
     * Reads a char value from the current position in the buffer.
     *
     * \param item The char value to read.
     *
     * \note If the parser is in an error state then no value will be read
     * from the buffer. If there is no space left in the buffer to read the value
     * then the parser will set to an error state.
     */
    void operator>>(char& item) { read_byte(item); }

    /**
     * Reads a uint8_t value from the current position in the buffer.
     *
     * \param item The uint8_t value to read.
     *
     * \note If the parser is in an error state then no value will be read
     * from the buffer. If there is no space left in the buffer to read the value
     * then the parser will set to an error state.
     */
    void operator>>(uint8_t& item) { read_byte(item); }

    /**
     * Reads an int8_t value from the current position in the buffer.
     *
     * \param item The int8_t value to read.
     *
     * \note If the parser is in an error state then no value will be read
     * from the buffer. If there is no space left in the buffer to read the value
     * then the parser will set to an error state.
     */
    void operator>>(int8_t& item) { read_byte(item); }

    /**
     * Reads a uint16_t value from the current position in the buffer in the
     * currently selected endianness.
     *
     * \param item The uint16_t value to read.
     *
     * \note If the parser is in an error state then no value will be read
     * from the buffer. If there is no space left in the buffer to read the value
     * then the parser will set to an error state.
     */
    void operator>>(uint16_t& item) { read(item); }

    /**
     * Reads an int16_t value from the current position in the buffer in the
     * currently selected endianness.
     *
     * \param item The int16_t value to read.
     *
     * \note If the parser is in an error state then no value will be read
     * from the buffer. If there is no space left in the buffer to read the value
     * then the parser will set to an error state.
     */
    void operator>>(int16_t& item) { read(item); }

    /**
     * Reads a uint32_t value from the current position in the buffer in the
     * currently selected endianness.
     *
     * \param item The uint32_t value to read.
     *
     * \note If the parser is in an error state then no value will be read
     * from the buffer. If there is no space left in the buffer to read the value
     * then the parser will set to an error state.
     */
    void operator>>(uint32_t& item) { read(item); }

    /**
     * Reads an int32_t value from the current position in the buffer in the
     * currently selected endianness.
     *
     * \param item The int32_t value to read.
     *
     * \note If the parser is in an error state then no value will be read
     * from the buffer. If there is no space left in the buffer to read the value
     * then the parser will set to an error state.
     */
    void operator>>(int32_t& item) { read(item); }

    /**
     * Reads a uint64_t value from the current position in the buffer in the
     * currently selected endianness.
     *
     * \param item The uint64_t value to read.
     *
     * \note If the parser is in an error state then no value will be read
     * from the buffer. If there is no space left in the buffer to read the value
     * then the parser will set to an error state.
     */
    void operator>>(uint64_t& item) { read(item); }

    /**
     * Reads an int64_t value from the current position in the buffer in the
     * currently selected endianness.
     *
     * \param item The int64_t value to read.
     *
     * \note If the parser is in an error state then no value will be read
     * from the buffer. If there is no space left in the buffer to read the value
     * then the parser will set to an error state.
     */
    void operator>>(int64_t& item) { read(item); }

    /**
     * Reads a float value from the current position in the buffer according to the
     * IEEE-754 standard. 4 bytes will be read from the buffer.
     *
     * \param item The float value to read.
     *
     * \note If the parser is in an error state then no value will be read
     * from the buffer. If there is no space left in the buffer to read the value
     * then the parser will set to an error state.
     */
    void operator>>(float_t& item);

    /**
     * Reads a double value from the current position in the buffer according to the
     * IEEE-754 standard. 8 bytes will be read from the buffer.
     *
     * \param item The double value to read.
     *
     * \note If the parser is in an error state then no value will be read
     * from the buffer. If there is no space left in the buffer to read the value
     * then the parser will set to an error state.
     */
    void operator>>(double_t& item);

    /**
     * Reads the ISomeIpSerializable object from the stream.
     * This calls the ISomeIpSerializable::parseFromArray method.
     *
     * \param item The serializable object to read.
     *
     * \note If the stream is in an error state then no value will be read
     * from the buffer. If an error happens during the call to parseFromArray then
     * the parser will be set to an error state. The data in the underlying object may
     * be corrupted and incomplete and should not be used.
     */
    void operator>>(ISomeIpSerializable& item);

    /**
     * Skips the specified number of bytes if there are that many bytes left.
     * If there are not enough bytes to skip then the parser is set to an error state.
     *
     * \param bytes The number of bytes to skip.
     */
    void skip(size_t bytes);

    uint32_t readTypeFieldSize();

    size_t getCurrentPosition() const { return _currentPos; }

    void resetCurrentPosition() { _currentPos = 0U; }

    /**
     * Returns how much more data can be read
     */
    size_t bytesAvailable() const { return _buffer.size() - _currentPos; }

    ::etl::span<uint8_t const> getAvailableBuffer() const { return _buffer.subspan(_currentPos); }

private:
    template<typename T>
    void read(T& item)
    {
        if (hasSpace(sizeof(T)) && isGood())
        {
            if (_bigEndian)
            {
                item = ::someip::endian::read_be<T>(&_buffer[_currentPos]);
            }
            else
            {
                item = ::someip::endian::read_le<T>(&_buffer[_currentPos]);
            }
            _currentPos += sizeof(T);
        }
    }

    template<typename T>
    void read_byte(T& item)
    {
        if (hasSpace(1) && isGood())
        {
            item = static_cast<T>(_buffer[_currentPos]);
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

    ::etl::span<uint8_t const> _buffer;
    size_t _currentPos;
};

void skip(SomeIpParser& parser, uint16_t tag);

} // namespace someip
