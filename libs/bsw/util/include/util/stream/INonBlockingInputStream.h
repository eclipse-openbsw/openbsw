// Copyright 2024 Accenture.

#ifndef GUARD_F92C2AA6_4256_494A_943C_4F4DE0BAD2D6
#define GUARD_F92C2AA6_4256_494A_943C_4F4DE0BAD2D6

#include <estd/slice.h>

#include <cstdint>

namespace util
{
namespace stream
{
/**
 * An input stream that delivers some or more read bytes.
 */
class INonBlockingInputStream
{
public:
    INonBlockingInputStream() = default;

    INonBlockingInputStream(INonBlockingInputStream const&)            = delete;
    INonBlockingInputStream& operator=(INonBlockingInputStream const&) = delete;

    /**
     * Indicates whether or not the stream ends i.e. reading makes sense.
     *
     * \return
     * - true if the end of the stream has been reached
     * - false otherwise
     */
    virtual bool isEof() const                                        = 0;
    /**
     * Reads a maximum number of bytes from the stream into a buffer.
     * \param buffer buffer that receives the bytes
     *
     * \return
     * number of bytes read into the buffer
     */
    virtual uint32_t readBuffer(::estd::slice<uint8_t> const& buffer) = 0;
};

} // namespace stream
} // namespace util

#endif /* GUARD_F92C2AA6_4256_494A_943C_4F4DE0BAD2D6 */
