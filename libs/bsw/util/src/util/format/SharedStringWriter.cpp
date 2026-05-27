/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "util/format/SharedStringWriter.h"

namespace util
{
namespace format
{
SharedStringWriter::SharedStringWriter(::util::stream::ISharedOutputStream& strm)
: StringWriter(strm.startOutput(nullptr)), _stream(strm)
{}

SharedStringWriter::~SharedStringWriter() { _stream.endOutput(nullptr); }

} // namespace format
} // namespace util
