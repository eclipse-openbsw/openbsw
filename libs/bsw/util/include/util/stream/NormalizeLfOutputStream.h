// Copyright 2024 Accenture.

#ifndef GUARD_4F4D1CB1_4346_427E_B849_C1AD7AA2FA53
#define GUARD_4F4D1CB1_4346_427E_B849_C1AD7AA2FA53

#include "util/stream/IOutputStream.h"

#include <estd/slice.h>

#include <cstdint>

namespace util
{
namespace stream
{
class NormalizeLfOutputStream : public IOutputStream
{
public:
    explicit NormalizeLfOutputStream(IOutputStream& strm, char const* crlf = nullptr);

    bool isEof() const override;

    void write(uint8_t data) override;
    void write(::estd::slice<uint8_t const> const& buffer) override;

private:
    IOutputStream& _stream;
    char const* _crlf;
};

} // namespace stream
} // namespace util

#endif /* GUARD_4F4D1CB1_4346_427E_B849_C1AD7AA2FA53 */
