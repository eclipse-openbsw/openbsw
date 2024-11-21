// Copyright 2024 Accenture.

#ifndef GUARD_8B59BA22_4005_4BE1_B81C_D12E1A736CD4
#define GUARD_8B59BA22_4005_4BE1_B81C_D12E1A736CD4

#include "util/stream/IOutputStream.h"

#include <estd/slice.h>

#include <cstdint>

namespace util
{
namespace stream
{
/**
 * The IOutputStream class that implements dummy output.
 *
 */
class NullOutputStream : public IOutputStream
{
public:
    bool isEof() const override;

    void write(uint8_t data) override;
    void write(::estd::slice<uint8_t const> const& buffer) override;
};

} // namespace stream
} // namespace util

#endif /* GUARD_8B59BA22_4005_4BE1_B81C_D12E1A736CD4 */
