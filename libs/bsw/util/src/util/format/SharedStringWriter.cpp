// Copyright 2024 Accenture.

#include "util/format/SharedStringWriter.h"

namespace util
{
namespace format
{
SharedStringWriter::SharedStringWriter(::util::stream::ISharedOutputStream& strm)
: StringWriter(strm.startOutput()), _stream(strm)
{}

SharedStringWriter::~SharedStringWriter() { _stream.endOutput(); }

} // namespace format
} // namespace util
