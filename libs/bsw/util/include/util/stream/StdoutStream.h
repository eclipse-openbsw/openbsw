// Copyright 2024 Accenture.

#ifndef GUARD_63A2535F_0A08_4032_91AE_B616A4B2350A
#define GUARD_63A2535F_0A08_4032_91AE_B616A4B2350A

#include "util/stream/IOutputStream.h"

namespace util
{
namespace stream
{
/**
 * The IOutputStream class implementation.
 *
 */
class StdoutStream : public IOutputStream
{
public:
    bool isEof() const override;

    void write(uint8_t data) override;
    void write(::estd::slice<uint8_t const> const& buffer) override;
};

} // namespace stream
} // namespace util

#endif /* GUARD_63A2535F_0A08_4032_91AE_B616A4B2350A */
