// Copyright 2024 Accenture.

#pragma once

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
