/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "logger/IEntryFormatter.h"
#include "logger/IEntryOutput.h"

#include <util/stream/NormalizeLfOutputStream.h>
#include <util/stream/StdoutStream.h>

namespace logger
{
template<class E = uint32_t, class Timestamp = uint32_t>
class ConsoleEntryOutput : public IEntryOutput<E, Timestamp>
{
public:
    explicit ConsoleEntryOutput(IEntryFormatter<E, Timestamp>& formatter);

    void outputEntry(
        E entryIndex,
        Timestamp timestamp,
        ::util::logger::ComponentInfo const& componentInfo,
        ::util::logger::LevelInfo const& levelInfo,
        char const* str,
        ::util::format::IPrintfArgumentReader& argReader) override;

private:
    ::util::stream::StdoutStream _stdout;
    ::util::stream::NormalizeLfOutputStream _stream;
    IEntryFormatter<E, Timestamp>& _formatter;
};

template<class E, class Timestamp>
ConsoleEntryOutput<E, Timestamp>::ConsoleEntryOutput(IEntryFormatter<E, Timestamp>& formatter)
: IEntryOutput<E, Timestamp>(), _stdout(), _stream(_stdout), _formatter(formatter)
{}

template<class E, class Timestamp>
void ConsoleEntryOutput<E, Timestamp>::outputEntry(
    E const entryIndex,
    Timestamp const timestamp,
    ::util::logger::ComponentInfo const& componentInfo,
    ::util::logger::LevelInfo const& levelInfo,
    char const* const str,
    ::util::format::IPrintfArgumentReader& argReader)
{
    _formatter.formatEntry(
        _stream, entryIndex, timestamp, componentInfo, levelInfo, str, argReader);
    _stream.write(static_cast<uint8_t>('\n'));
}

} // namespace logger
