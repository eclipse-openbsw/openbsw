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

#include "someip/SomeIpConstants.h"

#include <cstdint>

namespace someip
{
/**
 * A base class for read and writing Some/IP types.
 */
class SomeIpStreamer
{
public:
    SomeIpStreamer() : _errorCode(ErrorCode::SOMEIP_OK), _typeFieldSize(4U), _bigEndian(true) {}

    /**
     * Returns true if the underlying serializer or parser is in a good state.
     */
    bool isGood() const { return _errorCode == ErrorCode::SOMEIP_OK; }

    /**
     * Marks this stream as bad.
     */
    void setFailure() { _errorCode = ErrorCode::SOMEIP_ERROR; }

    /**
     * Sets the stream state to the specified error state.
     *
     * \param errorCode The error state.
     */
    void setFailure(ErrorCode const errorCode)
    {
        if (errorCode != ErrorCode::SOMEIP_OK)
        {
            _errorCode = errorCode;
        }
    }

    /**
     * Returns the current errors state.
     */
    ErrorCode getStatus() const { return _errorCode; }

    /** Sets the streamer encoding to big endian */
    void bigEndian() { _bigEndian = true; }

    /** Sets the streamer encoding to little endian */
    void littleEndian() { _bigEndian = false; }

    /** Returns the current type field size attribute */
    uint8_t getTypeFieldSize() const { return _typeFieldSize; }

    /** Sets the type field size attribute */
    void setTypeFieldSize(uint8_t const fieldSize) { _typeFieldSize = fieldSize; }

protected:
    ErrorCode _errorCode;
    uint8_t _typeFieldSize;
    bool _bigEndian;
};

void validate(SomeIpStreamer& streamer, bool result);

} // namespace someip
