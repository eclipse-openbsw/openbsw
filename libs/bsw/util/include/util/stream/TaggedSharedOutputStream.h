// Copyright 2024 Accenture.

#pragma once

#include "util/stream/ISharedOutputStream.h"
#include "util/stream/TaggedOutputHelper.h"

namespace util
{
namespace stream
{
/**
 * The TaggedOutputStream class is implementing the
 * ISharedOutputStream interface complemented with tagged output.
 *
 */
class TaggedSharedOutputStream
: public ISharedOutputStream
, private TaggedOutputHelper
, private IOutputStream
, private ISharedOutputStream::IContinuousUser
{
public:
    TaggedSharedOutputStream(
        ISharedOutputStream& sharedStream,
        char const* prefix,
        char const* suffix,
        bool continuous = false);
    ~TaggedSharedOutputStream();

    void release();

    IOutputStream& startOutput(ISharedOutputStream::IContinuousUser* user) override;
    void endOutput(ISharedOutputStream::IContinuousUser* user) override;
    void releaseContinuousUser(ISharedOutputStream::IContinuousUser& user) override;

private:
    bool isEof() const override;

    void write(uint8_t data) override;
    void write(::estd::slice<uint8_t const> const& buffer) override;

    void endContinuousOutput(IOutputStream& stream) override;

    ISharedOutputStream& _sharedStream;
    IOutputStream* _stream;
    ISharedOutputStream::IContinuousUser* _user;
    bool _continuous;
};

} // namespace stream
} // namespace util

