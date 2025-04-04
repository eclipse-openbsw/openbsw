// Copyright 2024 Accenture.

#pragma once

#include "estd/slice.h"
#include "estd/uncopyable.h"
#include "platform/estdint.h"
#include "uds/connection/ErrorCode.h"

namespace uds
{

/**
 * Class representing a UDS positive response.
 */
class PositiveResponse
{
    UNCOPYABLE(PositiveResponse);

public:
    PositiveResponse() {}
#ifdef UNIT_TEST
    virtual ~PositiveResponse(){};
#endif

    void init(uint8_t buffer[], size_t maximumLength);

    /**
     * Resets the response, i.e. the offset be set to zero.
     */
    void reset();

    /**
     * Appends a uint8_t to the response-buffer.
     */
    bool appendUint8(uint8_t data);

    /**
     * Appends a uint16_t to the response-buffer.
     */
    bool appendUint16(uint16_t data);

    /**
     * Appends a uint24_t (lower 3 bytes of uint32_t) to the response-buffer.
     */
    bool appendUint24(uint32_t data);

    /**
     * Appends a uint32_t to the response-buffer.
     */
    bool appendUint32(uint32_t data);

    /**
     * Appends a variable chunk of data to the response-buffer.
     *
     * \return    Number of bytes appended
     */
    size_t appendData(uint8_t const data[], size_t length);

    /**
     * Returns the size of the buffer passed to the init() function.
     */
    size_t getMaximumLength() const;

    /**
     * Returns the current length of the response.
     */
    size_t getLength() const;

    /**
     * Returns a writable pointer to the internal data array at the current
     * offset.
     */
    uint8_t* getData();

    /**
     * Returns the number of available bytes that can be appended to this response.
     */
    size_t getAvailableDataLength() const;

    /**
     * Increases the length of the response by a given \p length and advances the
     * internal buffer by that number.
     *
     * \warning
     * This function will assert if \p length exceeds getAvailableDataLength().
     */
    size_t increaseDataLength(size_t length);

    bool isOverflow() const;

private:
    ::estd::slice<uint8_t> fOriginalBuffer;
    ::estd::slice<uint8_t> fBuffer;
    bool fIsOverflow;
};

inline bool PositiveResponse::isOverflow() const { return fIsOverflow; }

} // namespace uds
