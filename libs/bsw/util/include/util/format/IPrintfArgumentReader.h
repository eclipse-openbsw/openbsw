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

#include "util/format/Printf.h"

namespace util
{
namespace format
{
/**
 * This interface is the base interface for providing data to a
 * ::util::format::PrintfFormatter.
 */
class IPrintfArgumentReader
{
public:
    IPrintfArgumentReader()          = default;
    virtual ~IPrintfArgumentReader() = default;

    IPrintfArgumentReader(IPrintfArgumentReader const&)            = delete;
    IPrintfArgumentReader& operator=(IPrintfArgumentReader const&) = delete;

    /**
     * Get the next argument value. The expected datatype is provided.
     * \param datatype expected datatype of the argument.
     * \return Pointer to the parameter value if existing. The provided value
     * is only valid up to the next call to readArgument, 0 otherwise
     */
    virtual ParamVariant const* readArgument(ParamDatatype datatype) = 0;
};

} // namespace format
} // namespace util
