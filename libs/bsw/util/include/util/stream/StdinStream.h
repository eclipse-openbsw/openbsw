// Copyright 2024 Accenture.

#ifndef GUARD_1AE114B1_E4D7_4465_A28F_C4780B9A88DF
#define GUARD_1AE114B1_E4D7_4465_A28F_C4780B9A88DF

#include "util/stream/INonBlockingInputStream.h"

namespace util
{
namespace stream
{
/**
 * The INonBlockingInputStream class implementation.
 *
 */
class StdinStream : public INonBlockingInputStream
{
public:
    bool isEof() const override;
    uint32_t readBuffer(::estd::slice<uint8_t> const& buffer) override;
};

} // namespace stream
} // namespace util

#endif /* GUARD_1AE114B1_E4D7_4465_A28F_C4780B9A88DF */
