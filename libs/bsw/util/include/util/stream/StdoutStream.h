// Copyright 2024 Accenture.

#pragma once

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

