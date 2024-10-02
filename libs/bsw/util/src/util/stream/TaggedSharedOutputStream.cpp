// Copyright 2024 Accenture.

#include "util/stream/TaggedSharedOutputStream.h"

#include <estd/memory.h>

namespace util
{
namespace stream
{
TaggedSharedOutputStream::TaggedSharedOutputStream(
    ISharedOutputStream& sharedStream,
    char const* const prefix,
    char const* const suffix,
    bool const continuous)
: TaggedOutputHelper(prefix, suffix)
, _sharedStream(sharedStream)
, _stream(nullptr)
, _user(nullptr)
, _continuous(continuous)
{}

TaggedSharedOutputStream::~TaggedSharedOutputStream() { release(); }

void TaggedSharedOutputStream::release()
{
    if (_continuous)
    {
        _sharedStream.releaseContinuousUser(*this);
        _continuous = false;
    }
}

IOutputStream&
TaggedSharedOutputStream::startOutput(ISharedOutputStream::IContinuousUser* const user)
{
    _stream = &_sharedStream.startOutput(_continuous ? this : nullptr);
    if ((!_continuous) || (user == nullptr) || (user != _user))
    {
        if (_user != nullptr)
        {
            _user->endContinuousOutput(*_stream);
        }
        endLine(*_stream);
    }
    _user = user;
    return *this;
}

void TaggedSharedOutputStream::endOutput(ISharedOutputStream::IContinuousUser* const user)
{
    if (user == nullptr)
    {
        endLine(*_stream);
    }
    _user   = user;
    _stream = nullptr;
    _sharedStream.endOutput(
        ((_user != nullptr) && _continuous && (!isLineStart())) ? this : nullptr);
}

void TaggedSharedOutputStream::releaseContinuousUser(ISharedOutputStream::IContinuousUser& user)
{
    if (_user == &user)
    {
        (void)startOutput(nullptr);
        endOutput(nullptr);
    }
}

bool TaggedSharedOutputStream::isEof() const { return (_stream == nullptr) || _stream->isEof(); }

void TaggedSharedOutputStream::write(uint8_t const data)
{
    if (_stream != nullptr)
    {
        writeBytes(*_stream, ::estd::memory::as_slice(&data));
    }
}

void TaggedSharedOutputStream::write(::estd::slice<uint8_t const> const& buffer)
{
    if (_stream != nullptr)
    {
        writeBytes(*_stream, buffer);
    }
}

void TaggedSharedOutputStream::endContinuousOutput(IOutputStream& stream) { endLine(stream); }

} // namespace stream
} // namespace util
