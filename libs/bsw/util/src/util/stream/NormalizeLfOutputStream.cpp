// Copyright 2024 Accenture.

#include "util/stream/NormalizeLfOutputStream.h"

namespace util
{
namespace stream
{
NormalizeLfOutputStream::NormalizeLfOutputStream(IOutputStream& strm, char const* const crlf)
: IOutputStream(), _stream(strm), _crlf((crlf != nullptr) ? crlf : "\r\n")
{}

bool NormalizeLfOutputStream::isEof() const { return _stream.isEof(); }

void NormalizeLfOutputStream::write(uint8_t const data)
{
    if (data == static_cast<uint8_t>('\n'))
    {
        char const* current = _crlf;
        while (*current != '\0')
        {
            _stream.write(static_cast<uint8_t>(*current));
            ++current;
        }
    }
    else
    {
        _stream.write(data);
    }
}

void NormalizeLfOutputStream::write(::estd::slice<uint8_t const> const& buffer)
{
    size_t const size = buffer.size();
    for (size_t idx = 0U; idx < size; ++idx)
    {
        write(buffer[idx]);
    }
}

} // namespace stream
} // namespace util
