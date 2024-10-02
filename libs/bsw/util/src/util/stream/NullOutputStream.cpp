// Copyright 2024 Accenture.

#include "util/stream/NullOutputStream.h"

namespace util
{
namespace stream
{
bool NullOutputStream::isEof() const { return true; }

void NullOutputStream::write(uint8_t const) {}

void NullOutputStream::write(::estd::slice<uint8_t const> const&) {}

} // namespace stream
} // namespace util
